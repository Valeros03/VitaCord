import asyncio
import websockets
import json
import base64
import hashlib
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import hashes

async def test():
    headers = {
        "Origin": "https://discord.com",
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"
    }

    private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
    public_key = private_key.public_key()
    pub_pem = public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo
    ).decode('utf-8')
    pub_key_str = "".join(pub_pem.split('\n')[1:-2])

    async with websockets.connect('wss://remote-auth-gateway.discord.gg/?v=2', additional_headers=headers) as ws:
        msg1 = await ws.recv()

        await ws.send(json.dumps({
            "op": "init",
            "encoded_public_key": pub_key_str
        }))

        msg2 = await ws.recv()
        j2 = json.loads(msg2)

        if j2['op'] == 'nonce_proof':
            encrypted_nonce = base64.b64decode(j2['encrypted_nonce'])
            nonce = private_key.decrypt(
                encrypted_nonce,
                padding.OAEP(
                    mgf=padding.MGF1(algorithm=hashes.SHA256()),
                    algorithm=hashes.SHA256(),
                    label=None
                )
            )
            nonce_hash = hashlib.sha256(nonce).digest()
            proof = base64.urlsafe_b64encode(nonce_hash).decode('utf-8').rstrip('=')
            await ws.send(json.dumps({
                "op": "nonce_proof",
                "proof": proof
            }))

            msg3 = await ws.recv()
            j3 = json.loads(msg3)
            if j3['op'] == 'pending_remote_init':
                print("===========================================")
                print("URL:", f"https://discord.com/ra/{j3['fingerprint']}")
                print("PLEASE SCAN WITH YOUR DISCORD APP ON YOUR PHONE")
                print("===========================================")

            try:
                while True:
                    msg4 = await ws.recv()
                    print("Recv:", msg4)
            except Exception as e:
                pass

asyncio.run(test())
