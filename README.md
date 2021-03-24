# PLATAFORMA DE DADOS METEOROLÓGICOS
Esse projeto tem por objetivo o desenvolvimento de uma plataforma de dados meteorológicos composta por estações meteorológicas, banco de dados e interface gráfica para análise e monitoramento. Espera-se com esse trabalho formar uma base de dados que permitirá a elaboração de estudos e previsão de geração de energia solar que podem ser aplicados para verificar a viabilidade de instalação de usinas solares, essa plataforma também poderá ser instalada junto de uma unidade geradora para auxiliar no monitoramento de geração, previsões e auxílio na tomada de decisões. Por se tratar de um projeto open source, almeja-se também contribuir com a redução de obstáculos e custos para disseminação e divulgação tecnológica desse tipo de tecnologia.

![Projeto](./projeto.jpg)

## ESTAÇÃO METEOROLÓGICA
![Baia de instrumentação](./baia-de-instrumentos.jpg)
Seguindo os conceitos de sensor autônomo e internet das coisas (IoT), foi projetada e construída uma estação meteorológica com microcontrolador ESP8266, programado em C/C++® no ambiente Arduino®, e inicialmente com os sensores de direção do vento, temperatura, umidade, pressão, pluviômetro e anemômetro, posteriormente serão implementados sensores de radiação solar direta e global, através de comunicação serial também é possível a implementação de sensores para monitorar a geração de energia, a estação processa os dados desses sensores, envia para internet através de conexão Wi-Fi® e também armazena localmente em cartão de memória.

Os esquemas elétricos para montagem da estação podem ser acessados no diretório [esquema-eletrico](github.com/romildodcm/plataforma-de-dados-meteorologicos/tree/main/esquema-eletrico) e os códigos para os microcontroladores estão no diretório [códigos](github.com/romildodcm/plataforma-de-dados-meteorologicos/tree/main/codigos), no projeto o circuito foi confeccionado usando uma placa ilhada, abaixo estão as tabelas com materiais necessários, esses sistemas foram integrados e montados na estrutura da estação, como pode ser visto nas imagens, os arquivos com as peças e montagem com o programa *Inventor* estão no diretório [estuturas](github.com/romildodcm/plataforma-de-dados-meteorologicos/tree/main/estuturas).

Tabela 1: Lista de componentes eletrônicos e alojamentos da Estação Meteorológica.
Quantidade | Componente | Valor unitário (R$)
------------ | ------------- | -------------
1 | Bateria Estacionária 7Ah | 109,90
1 | Painel Solar Fotovoltaico 30W | 189,00
1 | Controlador de Carga RTD1230 30A 12/24V | 170,00
2 | Conector MC4 Painel Solar | 10,00
2 | Metro cabo 4 mm² | 5,00
1 | Metro fio 2,5 mm² flexível | 2,00
3 | Metro Cabo flexível 26 AWG | 0,37
1 | Caixa Plástica PB-255/2 | 79,90
1 | Pluviômetro | 344,61
1 | Indicador de direção do vento | 284,63
1 | Anemômetro | 263,970
1 | Sensor de temperatura e umidade SHT20 | 297,23
1 | Alojamento para sensor e temperatura e umidade | 250,74
1 | RTC DS3231 | 22,50
1 | Sensor de pressão e temperatura BMP280 | 14,90
4 | Diodo 1N4007 | 0,10
4 | Borne Conector F 2EDGK-5,0 - 2 Vias | 1,99
1 | Borne Conector F 2EDGK-5,0 - 4 Vias | 3,70
4 | Borne Conector M 2EDGVC-5,0 4P 180º | 1,75
1 | Wemos ESP8266 | 50,00
1 | ATtiny85 DIP8 | 17,00
1 | Soquete DIP8 | 3,00
1 | Mini chave HH Alavanca SMTS102 2 Posições | 2,70
1 | Barra de Pinos 40 vias 11,2mm 180 graus | 1,00
1 | Barra de soquete header fêmea 40 vias 180 graus | 1,65
1 | Cartão microSD com adaptador SD 8GB | 29,9
2 | Capacitor eletrolítico 470uF | 0,50
5 | Capacitor cerâmico 100nF | 0,10
1 | Resistor 220 Ohm 1/4W | 0,10
2 | Resistor 330 Ohm 1/4W | 0,10
2 | Resistor 1k Ohm 1/4W | 0,10
1 | Resistor 10k Ohm 1/4W | 0,10
1 | Resistor 47k Ohm 1/4W | 0,10
1 | Placa padrão tipo ilha 10x15 mm | 14,00
1 | Peças impressas em 3D | 50,00


Tabela 2: Lista de componentes da estrutura da estação meteorológica*.
Quantidade | Componente | Valor unitário (R$)
------------ | ------------- | -------------
1 | 2,5m Tubo redondo alumínio 1.1/4" X 1/16" (31,75mm X 1,58mm) | 39,00
1 | 3m Tubo quadrado alumínio 3/4 X 1/16 (1,9cm X 1,58mm) | 25,00
1 | Chapa de aço 2mmx80x60mm | 5,00
1 | Haste De Cobre Puro 10 mm X 500 mm | 50,00
1 | borracha espessura 1x80x20mm | 5,00
7 | Parafuso m3 x 15mm | 0,12
3 | Parafuso m3 x 25mm | 0,25
1 | Parafuso m5 x 25mm | 0,30
3 | Espaçador m3 x 15mm | 0,30
2 | Barra roscada m6 ou 3/8 x 100mm | 2,50
2 | Parafuso sextavado m6 ou 3/8 x 16mm | 0,30
4 | Parafuso sextavado m6 ou 3/8 x 30mm | 0,30
28 | Porca sextavada m6 ou 3/8 x 30mm | 0,10
28 | ruela de metal m6 ou 3/8 | 0,15
6 | ruela de borracha m6 ou 3/8 | 0,15
4 | Parafuso Sextavado Phillips NC9/64 x 1/4 | 0,10
3 | Abraçadeira Tipo "U" - 35 Fita 16 Aço Carbono | 9,10
4 | Abraçadeira Tipo "U" - 35 Fita 16 Aço Carbono 58mm comprimento | 11,15

*Observações:
* Na lista e no projeto não estão inclusos estrutura para fixação do painel solar, essa pode ser construída ou comprada, variando de acordo com o tipo de instalação, como pode ser visto na imagem, aqui ela foi fixada em uma cantoneira metálica e essa foi parafusada sobre a cobertura da casa;
* Essa lista é uma base que bate com o projeto disponibilizado no diretório de [estruturas](github.com/romildodcm/plataforma-de-dados-meteorologicos/tree/main/estuturas), podendo/devendo ser alterado em função do local de instalação e conforme os sensores que serão implementados, como se pode notar ao comparar a estação do projeto e a que foi montada, como a que está construída está sobre uma casa onde não precisa de para-raios (já tem em outra estrutura nas proximidades), e também ainda não foram implementados sensores solares, então a estrutura ficou mais simples, para economizar, também podem construir utilizando sucatas;
* Como pode ser visto na imagem a seguir, à esquerda o número 1 indica haste adicionada ao projeto para instalação de sensores solares, na estação construída (à direita) não tem essa haste pois esses sensores ainda não ficaram prontos para serem implementados. Outro detalhe são o uso das barras roscadas que aparecem no projeto e na estação montada, elas servem para prender e regular os fios que fixam e mantem o posicionamento da estação.

![Projeto](./projeto-2.png)

## BANCO DE DADOS E INTERFACE GRÁFICA

![Dashboard Grafana](./grafana.png)

O banco de dados e a interface de usuário foram implementados em um Raspberry Pi® utilizado como servidor, onde um algoritmo em Python® faz o recebimento e processamento dos dados que são colocados em banco InfluxDB®, nesse servidor foi implementada com Grafana® uma interface gráfica que é acessada por usuários através de um navegador web, sendo apresentados os parâmetros atualizados e séries temporais, como mostra a figura acima. No diretório [codigos/raspberry_pi](github.com/romildodcm/plataforma-de-dados-meteorologicos/tree/main/codigos/raspberry_pi) é possível ver o script implementado e a documentação que apresenta como fazer a implementação dessa parte do projeto.

Tabela 3: Lista de Materiais para o Servidor.
Quantidade | Componente | Valor unitário (R$)
------------ | ------------- | -------------
1 | Raspberry Pi 4 Model B 4GB | 640,00
1 | Fonte para Raspberry Pi 4 | 65,00
1 | Cartão de memória classe 10 16GB | 60,00

## EQUIPE

EU E PROFESSOR + CONTATO

## APOIO E FOMENTO

LOGO UNILA GEPENSE CNPQ

## AGRADECIMENTOS
FAMÍLIA, AMIGOS, PROFESSORES, JOYLAN E LUCAS TESKE










**Essas informações aqui podem ser retiradas do pré-artigo e das listas de compras**
- [ ] Introdução, print de interface, etc;
- [ ] Lista de materiais + valores estimados e comparação com preço de uma estação;
- [ ] Explicação de como instala programas necessários e links para diretórios com os códigos, lembrar que a explicação pode ter "tal coisa usa isso, acesse aqui para ver como fazer a instalação";
- [ ] Comentar que tem projeto da estação para instalar no telhado ou em solo, colocar renderizações do projeto e link para o diretório com os desenhos técnicos;
- [ ] Dados de contato e licença;
- [ ] Agradecimentos;

configuração grafana, influx, etc
https://diyi0t.com/visualize-mqtt-data-with-influxdb-and-grafana/
ver se implemento a opção de adicionar senhas, etc.

LOGO UNILA, GEPEN, CNPQ


