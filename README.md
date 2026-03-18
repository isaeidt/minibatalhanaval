# Batalha Naval em Rede

### Mini Projeto - Sistemas Operacionais

#### Instituto Federal de Santa Catarina – Análise e Desenvolvimento de Sistemas 
--- 
* **Professor:**  Eraldo Silveira e Silva
* **Alunos:**

  * Haydee Dela Vedova Murara
  * Isadora Soares Eidt
  * Sant Semeghini

<br>

## 1. Diagrama de arquitetura  
``` 
                  +----------------------+
                |   Cliente (bot)      |
                |  (curl / estratégia) |
                +----------+-----------+
                           |
                           | HTTP (GET /status, /tiro)
                           v
                +----------------------+
                |   Mini Web Server    |
                |  (processo principal)|
                +----------+-----------+
                           |
                           | FIFO (IPC local)
                           v
        +--------------------------------------+
        |   Processos dos Navios (fork)        |
        |  Navio1  Navio2 ... Navio6           |
        +--------------------------------------+
```

## 2.  Mecanismo IPC local
Foi utilizado FIFO (Named Pipe) para comunicação entre processos.

### 2.1 Funcionamento:
  * Os navios (processos filhos) escrevem na FIFO
  * O servidor lê continuamente essa FIFO

### 2.2 Características:
  * Comunicação unidirecional
  * Baseada em sistema de arquivos
  * Permite troca de dados entre processos independentes

--- 

## 3.  Estrutura de dados
Os navios são representados por uma Struct:
``` C
typedef struct {
    int id; // Identificador único
    char tipo[50]; // Tipo do navio (fragata, submarino, etc.)
    int linha;   //
    int coluna;  // Posição atual
    int ativo; // Indica se o navio ainda está no jogo
    int pid; // ID do processo que é usado no kill
    int pontos; // Valor ao ser destruído
} Navio;
```
---

## 4.  Formato dos dados internos 
Os navios enviam dados via FIFO, os quais são interpretados pelo servidor e usados para atualizar o status do jogo. 
```

id,tipo,linha,coluna // EX: 3,submarino,2,5

```
---

## 5.  Endpoints HTTP
### 5.1 `/status`
``` json
[
  {"id":1,"tipo":"porta_avioes","linha":0,"coluna":3},
  {"id":2,"tipo":"submarino","linha":1,"coluna":7}
]
```

### 5.2 `/tiro?linha=X&coluna=Y`
``` json
//Acerto 
{"resultado":"acerto","tipo":"fragata","pontos":2}

//Água
{"resultado":"agua"}
```
---

## 6.  Estratégia de ataque
### 6.1 Passos
  1. Consulta `/status` para identificar linhas com navios
  2. Seleciona apenas essas linhas
  3. Realiza ataques em todas as colunas dessas linhas
     
### 6.2 Vantagens
  1. Reduz tiros desnecessários
  2. Garante encontrar os navios

### 6.3 Desvantagens
  1. Não para após acerto
  2. Não prioriza alvos
  3. Desperdiça tiros

---

## 7.  Dificuldades encontradas
  1. **Encerramento dos processos `(kill)`:**
     Garantir que um navio fosse realmente removido do sistema após um acerto exigiu o uso adequado de sinais `(SIGKILL)` e o correto mapeamento entre navio e processo.
     
  2. **Consistência do estado do jogo:**
     Foi necessário ter atenção a atualização e sincronização das posições dos navios para evitar erros e inconsistências.

  3. **Compatibilidade de ambiente:**
     Algumas bibliotecas utilizadas como `unistd.h` não são nativas do Windows, e foram necessários ajustes nos ambientes de trabalho para alguns integrantes do grupo.

---

## 8.  Decisões do projeto
  1. **`SIGKILL` para encerrar navios:**
     Garante o término imediato e evita que o navio continue enviando dados.
     
  2. **Formato textual na FIFO `(id,tipo,linha,coluna)`:**
     Formato textual simples para facilitar a implementação.

  3. **Estratégia de ataque baseada em `/status`:**
     Abordagem que primeiro coleta informações e depois ataca, ao invés de tiros aleatórios. Acabou não sendo a mais eficaz mas garantiu resultados consistentes.
