# Port Scanner (C / Winsock2)

Scanner de portas TCP em C para Windows, usando sockets não-bloqueantes (Winsock2) para verificar o estado de portas em um endereço IP especificado.

## Funcionalidades

- Escaneia um intervalo de portas TCP em um IP informado pelo usuário
- Detecta portas **abertas**, **fechadas** e **filtradas**
- Timeout configurável por porta (atualmente 200ms)
- Validação de entrada (IP e intervalo de portas)
- Menu interativo com opção de sair

## Pré-requisitos

- Windows
- Compilador GCC (via MinGW) instalado e configurado no PATH
- Biblioteca Winsock (`ws2_32`), incluída no Windows por padrão

## Como compilar

```
gcc "Port scanner.c" -o port_scanner.exe -lws2_32
```

## Como usar

1. Execute `port_scanner.exe`
2. No menu, escolha a opção `1` para iniciar o scanner
3. Digite o IP a ser escaneado (ex: `8.8.8.8`)
4. Digite o intervalo de portas separado por espaço (ex: `1 1000`)
5. O programa exibirá as portas abertas encontradas individualmente, e um resumo de portas fechadas/filtradas ao final
6. Escolha `0` no menu para sair

## Limitações conhecidas

- O tempo de scan cresce linearmente com o tamanho do intervalo de portas, já que cada porta é testada sequencialmente com timeout de 200ms. Uma versão futura com múltiplas threads ou I/O assíncrono em lote poderia paralelizar esse processo.
- A validação do IP ocorre a cada porta escaneada, então um IP inválido gera uma mensagem de erro repetida por porta no intervalo, em vez de uma única mensagem antes do scan começar.
- Um firewall filtra a maioria das portas por padrão, então a maior parte dos resultados tende a aparecer como filtrada. Em portas realmente abertas e em modo de escuta (listening), como em endereços de loopback (`127.0.0.1`), o scanner funciona normalmente e identifica o estado corretamente.
- O tratamento de erros na leitura de IP e portas é mínimo: cobre falha de leitura e intervalo de portas fora dos limites, mas não valida todos os casos possíveis de entrada malformada.

## Arquitetura

Fluxo principal do programa:

```
main → menuScanner → buildSocket (por porta) → socketCreate + socketConnect → scanner (decide o estado da porta)
```

- **`socketCreate`**: cria o socket e configura como não-bloqueante
- **`socketConnect`**: preenche as informações de destino e inicia a conexão
- **`scanner`**: usa `select()` para verificar se a conexão foi concluída dentro do timeout, e classifica a porta como aberta, fechada ou filtrada
- **`buildSocket`**: organiza a sequência criar → conectar → escanear para uma porta
- **`menuScanner`**: coleta IP e intervalo de portas do usuário, executa scan e exibe o resumo final
- **`requestIp`**: lê o IP digitado
