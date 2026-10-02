# Blood Strike Overlay

Projeto simples em C++/Win32 com janela overlay transparente e arrastável, pronta para compilar no Visual Studio 2022.

Objetivo:
- janela topmost sobre o jogo
- toggle de opções em menu
- suporte a arraste da janela
- tecla `Insert` para esconder/mostrar a janela

Resumo da estrutura:
- `BloodStrikeOverlay.sln`
- `BloodStrikeOverlay/BloodStrikeOverlay.vcxproj`
- `BloodStrikeOverlay/main.cpp`

Como compilar:
1. Abra o arquivo `BloodStrikeOverlay.sln` no Visual Studio 2022.
2. Selecione `x64` e `Release`.
3. Build -> Build Solution.
4. Execute o `.exe` gerado em `bin\Release\`.

Observação:
Este projeto é um overlay de ferramentas de desenvolvimento para um jogo próprio e não faz injeção de DLL nem manipulação de processo de terceiros.
