import binascii as ba
import pysodium as nacl
import msgpack
from struct import unpack
import sys

key = sys.argv[1]
msg = sys.argv[2]

mp = ba.unhexlify(msg)
frame = msgpack.loads(mp)
print(frame)
cipher = frame[b'data']
print(len(cipher))
print(ba.hexlify(cipher))
nonce = frame[b'nonce']
print(len(nonce))
print(ba.hexlify(nonce))
data = nacl.crypto_aead_chacha20poly1305_ietf_decrypt(cipher, None, nonce, key)
print(ba.hexlify(data))
print(unpack('hHHH', data))
