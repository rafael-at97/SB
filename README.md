# SB
Repositório para os trabalhos de SB

Intergrantes do Grupo:
	Rafael Torres  - Matricula 15/0145365
	Thiago Holanda - Matricula 13/0018007

Sistema Operacional:
	Ubuntu 16.04.5 LTS
Versão do G++:
	INSERIR VERSÂO DO G++
Versão do C++:
	Escrito na versão std 98

Como compilar os programas:
	Coloque todos os arquivos do tipo .cpp e .h com o arquivo makefile em uma mesma pasta e digite o comando:
		"make all"
	Os programas do montador e ligador estarão em uma pasta chamada simuladores no mesmo diretório onde foram colocados os arquivos.	
	
	Para montar algum arquivo do tipo .asm, simplesmente digite:
		"./montador file.asm"
	onde file é o nome do seu arquivo
	Para o montador pode utilizar os arquivos sem extensão, ele apontará como erro mas assumirá a extensão .asm e irá montar normalmente
	
	Todas as definições com a diretiva EQU devem vir no começo do código antes de qualquer outra instrução e/ou diretivas.	
		Caracteristicas do montador:
		-Não é sensível ao caso
		-Não tem ordem fixa das seções BSS e DATA, seção TEXT precisa ser a primeira
		-Arquivo de saída do tipo texto com extensão .obj
		-Desconsidera tabulações, quebras de linha e espaços desnecessários
		-CONST aceita números positivos e negativos
		-Possível trabalhar com vetores
		-Aceita comentários sendo iniciado com ;
	Erros detectados pelo montador:
		– declarações e rótulos ausentes;
		– declarações e rótulos repetidos;
		– pulo para rótulos inválidos;
		– pulo para seção errada;
		– diretivas inválidas; O montador permite jumps para diretiva begin mesmo apontando erro.
		– instruções inválidas;
		– diretivas ou instruções na seção errada;
		– divisão por zero (para constante);
		– instruções com a quantidade de operando inválida;
		– tokens inválidos;
		– dois rótulos na mesma linha;
		– seção TEXT faltante;
		– seção inválida;
		– tipo de argumento inválido;
		– modificação de um valor constante;
	
	Para rodar o ligador do tipo .obj, simplesmente digite:
		"./ligador file_1.obj file_2.obj file_3.obj file_4.obj"
	onde os file_x são os nomes dos arquivos a serem ligados
	Para o ligador pode utilizar os arquivos sem extensão, ele apontará como erro mas assumirá a extensão .obj e irá ligar normalmente
