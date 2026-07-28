# 🌐 .KAMELIA - Rede Anônima Descentralizada

**Uma rede anônima do zero, inspirada no Tor, mas com implementação própria.**
      
---

## 🔥 O que é?

**.KAMELIA** é uma rede descentralizada que permite comunicação anônima entre nós. Cada nó tem um endereço único (ex: `abc123.kamelia`) e a comunicação é roteada através de múltiplos relays, garantindo que nenhum nó saiba a origem e o destino final da mensagem.

---

## 🧠 Como funciona?                        .

```
Cliente → Relay 1 → Relay 2 → Relay 3 → Servidor
              ↓          ↓          ↓
           Reencaminha  Reencaminha  Reencaminha
              ↓          ↓          ↓
Cliente ← Relay 1 ← Relay 2 ← Relay 3 ← Servidor
```

- Cada relay só conhece o anterior e o próximo
- Ninguém sabe a história completa
- O anonimato é construído camada por camada

---

## ✅ O que já está funcionando

| Funcionalidade | Status |
|----------------|--------|
| Geração de chaves Ed25519 | ✅ |
| Endereço .KAMELIA (Base64) | ✅ |
| Servidor UDP (PING/PONG) | ✅ |
| Relay com reencaminhamento | ✅ |
| Relay com memória (resposta volta) | ✅ |
| Cadeia de 3 relays | ✅ |

---

## 🚧 O que falta implementar

| Funcionalidade | Prioridade |
|----------------|------------|
| **Onion Routing (cifra em camadas)** | 🔥 Alta |
| **Serviço HTTP (site .KAMELIA)** | 🔥 Alta |
| **DHT (descoberta de nós)** | 🟡 Média |
| **Cliente automático** | 🟡 Média |
| **Hole punching (NAT traversal)** | 🟢 Baixa |

---

## 🛠️ Como testar

### 1. Gerar chaves e endereço
```bash
./kamelia gen
```

### 2. Rodar servidor
```bash
./kamelia server
```

### 3. Rodar relays
```bash
./kamelia relay 5000 5001
./kamelia relay 5001 5002
./kamelia relay 5002 9999
```

### 4. Testar com cliente
```bash
echo -n "PING" | nc -u -w 2 localhost 5000
```

---

## 📁 Estrutura do projeto

```
kamelia/
├── include/
│   └── kamelia.h          # Headers
├── src/
│   ├── main.c             # Entry point
│   ├── kamelia.c          # Funções principais
│   ├── relay.c            # Relay
│   └── node/
│       ├── addr.c         # Geração de endereço
│       ├── keys.c         # Geração de chaves
│       └── udp.c          # Socket UDP
├── keys/                  # Chaves geradas
│   ├── PUBLIC_KEY.kam
│   ├── PRIVATE_KEY.kam
│   └── hostname
└── README.md
```

---

## 📦 Dependências

- **libsodium** (criptografia)
- **gcc** (compilador)
- **netcat** (para testes)

Instalar no Ubuntu/Debian:
```bash
sudo apt install libsodium-dev gcc netcat-openbsd
```

Compilar:
```bash
gcc -o kamelia src/main.c src/kamelia.c src/relay.c src/node/*.c -lsodium
```

---

## 🧅 Próximo passo: Onion Routing

O próximo grande passo é implementar cifra em camadas:

```
Cliente cifra mensagem com chave do Relay 3
        cifra com chave do Relay 2
        cifra com chave do Relay 1
        envia para Relay 1

Relay 1 decifra sua camada → vê próximo destino
Relay 2 decifra sua camada → vê próximo destino
Relay 3 decifra sua camada → vê mensagem original
```

**Nenhum relay vê a mensagem completa!**

---

## 📝 Licença

MIT

---

## 🙌 Contribuições

Este é um projeto educacional. Contribuições são bem-vindas!

---

**Feito com 💀 e café (chá)**
