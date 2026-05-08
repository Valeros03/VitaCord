import socket

# Parametri di ascolto
UDP_IP = "0.0.0.0"  # 0.0.0.0 significa "Ascolta su tutte le interfacce di rete del PC"
UDP_PORT = 18194    # Questa è la porta di default usata da debugNet. 

def main():
    # Creazione del socket UDP
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))

    print(f"🎧 In ascolto dei log di debugNet sulla porta UDP {UDP_PORT}...")
    print("Premi Ctrl+C per fermare lo script.\n")
    print("-" * 50)

    try:
        while True:
            # Riceve i dati (buffer di 4096 byte) e l'indirizzo IP della PS Vita
            data, addr = sock.recvfrom(4096)
            
            # Decodifica i byte in testo (ignora eventuali caratteri strani)
            message = data.decode('utf-8', errors='replace').strip()
            
            # Stampa l'IP della Vita e il log
            print(f"[{addr[0]}] {message}")
            
    except KeyboardInterrupt:
        print("\n\n🛑 Ascolto terminato.")
    finally:
        sock.close()

if __name__ == "__main__":
    main()
