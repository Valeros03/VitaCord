#include "VitaCordStreamer.h"
#include <psp2/kernel/threadmgr.h>
#include <debugnet.h>
#include <string.h>
#include <stdio.h>

VitaCordStreamer::VitaCordStreamer(int port) 
    : RECV_PORT(port), udp_socket(-1), running(false), 
      decoderMemUID(-1), pDecoderMem(nullptr),
      outputFrameMemUID(-1), current_write_idx(0), current_read_idx(1) {
    
    pOutputFrameMem[0] = nullptr;
    pOutputFrameMem[1] = nullptr;

    // Inizializza il mutex
    pthread_mutex_init(&frame_mutex, NULL);
}
VitaCordStreamer::~VitaCordStreamer() {
    Stop();
    // Distruggi il mutex
    pthread_mutex_destroy(&frame_mutex);
}

bool VitaCordStreamer::Initialize() {
    if (!SetupDecoder()) return false;
    if (!SetupNetwork()) return false;
    return true;
}

bool VitaCordStreamer::SetupNetwork() {
    udp_socket = sceNetSocket("VitaCord_UDP", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, 0);
    if (udp_socket < 0) return false;

    SceNetSockaddrIn server_addr;
    server_addr.sin_family = SCE_NET_AF_INET;
    server_addr.sin_addr.s_addr = sceNetHtonl(SCE_NET_INADDR_ANY);
    server_addr.sin_port = sceNetHtons(RECV_PORT);

    if (sceNetBind(udp_socket, (SceNetSockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        sceNetSocketClose(udp_socket);
        return false;
    }

    // Imposta socket non bloccante per poter uscire pulitamente dal thread
    int optval = 1;
    sceNetSetsockopt(udp_socket, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &optval, sizeof(optval));
    return true;
}
bool VitaCordStreamer::SetupDecoder() {
    debugNetPrintf(DEBUG, "[STREAMER] Avvio SetupDecoder...\n");

    SceAvcdecQueryDecoderInfo queryDecoderInfo;
    memset(&queryDecoderInfo, 0, sizeof(queryDecoderInfo));
    queryDecoderInfo.horizontal = FRAME_WIDTH;
    queryDecoderInfo.vertical = FRAME_HEIGHT;
    queryDecoderInfo.numOfRefFrames = 3;

    SceAvcdecDecoderInfo decoderInfo;
    debugNetPrintf(DEBUG, "[STREAMER] Chiamo sceAvcdecQueryDecoderMemSize...\n");
    int res = sceAvcdecQueryDecoderMemSize(SCE_VIDEODEC_TYPE_HW_AVCDEC, &queryDecoderInfo, &decoderInfo);
    debugNetPrintf(DEBUG, "[STREAMER] sceAvcdecQueryDecoderMemSize: 0x%08X (Richiesti %d byte)\n", res, decoderInfo.frameMemSize);
    if (res < 0) return false;

    debugNetPrintf(DEBUG, "[STREAMER] Alloco memoria fisica...\n");
    decoderMemUID = sceKernelAllocMemBlock("AvcDecMem", 
        SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, 
        (decoderInfo.frameMemSize + 0xFFFFF) & ~0xFFFFF, nullptr);
    debugNetPrintf(DEBUG, "[STREAMER] decoderMemUID: 0x%08X\n", decoderMemUID);
    if (decoderMemUID < 0) return false;
    
    sceKernelGetMemBlockBase(decoderMemUID, &pDecoderMem);

    memset(&decoderCtrl, 0, sizeof(SceAvcdecCtrl));
    decoderCtrl.frameBuf.pBuf = pDecoderMem;
    decoderCtrl.frameBuf.size = decoderInfo.frameMemSize;
    
    debugNetPrintf(DEBUG, "[STREAMER] Chiamo sceAvcdecCreateDecoder...\n");
    res = sceAvcdecCreateDecoder(SCE_VIDEODEC_TYPE_HW_AVCDEC, &decoderCtrl, &queryDecoderInfo);
    debugNetPrintf(DEBUG, "[STREAMER] sceAvcdecCreateDecoder: 0x%08X\n", res);
    if (res < 0) return false;

    debugNetPrintf(DEBUG, "[STREAMER] Decoder creato. Alloco OutputFrameMem...\n");

    // 4. Allocazione Memoria di Output (Double Buffering RGBA8888)
    // Richiede allineamento a 256 byte. Un frame 480x272x4 = ~522 KB. Alloco 2MB per sicurezza.
    outputFrameMemUID = sceKernelAllocMemBlock("AvcFrameMem", 
        SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, 
        2 * 1024 * 1024, nullptr);
    if (outputFrameMemUID < 0) return false;
    
    void* pBaseOutputMem;
    sceKernelGetMemBlockBase(outputFrameMemUID, &pBaseOutputMem);
    
    // Splitto il blocco a metà per i due buffer
    pOutputFrameMem[0] = pBaseOutputMem;
    pOutputFrameMem[1] = (uint8_t*)pBaseOutputMem + (1024 * 1024);

    return true;
}

void VitaCordStreamer::Start() {
    if (running) return;
    running = true;
    
    debugNetPrintf(DEBUG, "[STREAMER] Creazione pthread in corso...\n");
    pthread_create(&recv_thread, NULL, &VitaCordStreamer::ReceiveThreadWrapper, this);
}

void* VitaCordStreamer::ReceiveThreadWrapper(void* arg) {
    VitaCordStreamer* streamer = static_cast<VitaCordStreamer*>(arg);
    debugNetPrintf(DEBUG, "[STREAMER] Pthread avviato con successo! Entro in ReceiveLoop...\n");
    streamer->ReceiveLoop();
    return NULL;
}

void VitaCordStreamer::Stop() {
    if (!running) return;
    running = false;
    
    // Attende che il thread finisca
    pthread_join(recv_thread, NULL);

    if (udp_socket >= 0) {
        sceNetSocketClose(udp_socket);
        udp_socket = -1;
    }
    // Distruzione decoder
    sceAvcdecDeleteDecoder(&decoderCtrl);

    // Free della memoria fisica
    if (decoderMemUID >= 0) sceKernelFreeMemBlock(decoderMemUID);
    if (outputFrameMemUID >= 0) sceKernelFreeMemBlock(outputFrameMemUID);
}

void VitaCordStreamer::ReceiveLoop() {
    uint8_t recv_buffer[2048];
    SceNetSockaddrIn client_addr;
    unsigned int addr_len = sizeof(client_addr);

    while (running) {
        int bytes_received = sceNetRecvfrom(udp_socket, recv_buffer, sizeof(recv_buffer), 0, 
                                            (SceNetSockaddr*)&client_addr, &addr_len);

        if (bytes_received > 0) {
            au_buffer.insert(au_buffer.end(), recv_buffer, recv_buffer + bytes_received);
        } else {
            // Socket non bloccante: se non ci sono dati, fai riposare il thread per 1ms
            sceKernelDelayThread(1000); 
        }

        if (au_buffer.size() > 0 && HasFullAccessUnit(au_buffer)) {
            SceAvcdecAu au;
            memset(&au, 0, sizeof(SceAvcdecAu));
            au.pts.upper = 0xFFFFFFFF;
            au.pts.lower = 0xFFFFFFFF;
            au.dts.upper = 0xFFFFFFFF;
            au.dts.lower = 0xFFFFFFFF;
            au.es.pBuf = au_buffer.data();
            au.es.size = au_buffer.size();

            SceAvcdecPicture outPicture;
            memset(&outPicture, 0, sizeof(SceAvcdecPicture));
            outPicture.size = sizeof(SceAvcdecPicture);
            outPicture.frame.pixelType = SCE_AVCDEC_PIXELFORMAT_RGBA8888;
            outPicture.frame.framePitch = FRAME_WIDTH;
            outPicture.frame.frameWidth = FRAME_WIDTH;
            outPicture.frame.frameHeight = FRAME_HEIGHT;
            
            // Scriviamo nel buffer "nascosto"
            outPicture.frame.pPicture[0] = pOutputFrameMem[current_write_idx];
            outPicture.frame.pPicture[1] = nullptr;

            SceAvcdecPicture* pOutPicturePtr = &outPicture;
            SceAvcdecArrayPicture arrayPicture;
            arrayPicture.numOfElm = 1;
            arrayPicture.pPicture = &pOutPicturePtr;

            int res = sceAvcdecDecode(&decoderCtrl, &au, &arrayPicture);

            if (res == SCE_AVCDEC_ERROR_ES_BUFFER_FULL) {
                // Buffer hardware pieno, riproviamo al prossimo giro
                continue;
            }

            if (res == 0 && arrayPicture.numOfOutput == 1) {
                // Decodifica riuscita! Facciamo lo SWAP dei buffer in modo thread-safe
                pthread_mutex_lock(&frame_mutex);
                int temp = current_read_idx;
                current_read_idx = current_write_idx;
                current_write_idx = temp;
                pthread_mutex_unlock(&frame_mutex);
                
                au_buffer.clear();
            } else if (res < 0) {
                // Frame corrotto, droppiamo per evitare artefatti
                au_buffer.clear();
            }
        }
    }
}

bool VitaCordStreamer::HasFullAccessUnit(const std::vector<uint8_t>& buffer) {
    // Implementazione base: cerca i delimitatori NALU (0x00 0x00 0x00 0x01)
    // Per un Elementary Stream robusto, dovrai implementare una logica che verifica
    // che ci siano due start code, indicando la fine del frame attuale.
    // Per ora, come placeholder, ritorniamo true se c'è abbastanza payload.
    if (buffer.size() > 4096) return true; 
    return false;
}

void* VitaCordStreamer::GetLatestFrame() {
    pthread_mutex_lock(&frame_mutex);
    void* frame = pOutputFrameMem[current_read_idx];
    pthread_mutex_unlock(&frame_mutex);
    
    return frame;
}