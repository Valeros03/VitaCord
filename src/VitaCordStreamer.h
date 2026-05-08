#pragma once

#include <psp2/net/net.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/videodec.h>
#include <vector>
#include <pthread.h>
#include <atomic>

class VitaCordStreamer {
public:
    // Costruttore: accetta la porta UDP su cui ascoltare
    VitaCordStreamer(int port);
    ~VitaCordStreamer();

    // Inizializza memoria hardware e socket
    bool Initialize();
    
    // Avvia il thread di ricezione
    void Start();
    
    // Ferma il thread e pulisce in modo sicuro
    void Stop();

    // Funzione che la GUI chiamerà ad ogni frame per ottenere la texture decodificata
    // Restituisce nullptr se non ci sono nuovi frame pronti
    void* GetLatestFrame();

private:
    // Costanti
    const int RECV_PORT;
    const int FRAME_WIDTH = 480;
    const int FRAME_HEIGHT = 272;

    // Rete
    int udp_socket;
    
    // Threading
    pthread_t recv_thread;
    pthread_mutex_t frame_mutex;
    std::atomic<bool> running;
    
    // Decoder Hardware
    SceAvcdecCtrl decoderCtrl;
    SceUID decoderMemUID;
    void* pDecoderMem;

    // Double Buffering per la GUI (evita il tearing)
    SceUID outputFrameMemUID;
    void* pOutputFrameMem[2]; // Due buffer per lo swap
    int current_write_idx;    // Buffer su cui scrive il decoder
    int current_read_idx;     // Buffer da cui legge la GUI

    // Buffer per ricostruire l'Access Unit (AU) H.264
    std::vector<uint8_t> au_buffer;

    // Metodi interni
    bool SetupNetwork();
    bool SetupDecoder();
    void ReceiveLoop();
    bool HasFullAccessUnit(const std::vector<uint8_t>& buffer);
    static void* ReceiveThreadWrapper(void* arg);
};