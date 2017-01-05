// Gabriel M F Correia 31371477
/////////////////////////////////

#include<stdio.h>
#include <stdbool.h>
// Variavel matriz recebe o sudoku
//////////////////////////////////
int matriz[9][9];

// Contador de N de passos usados
//////////////////////////////////
int passos = 0;
int verif = 10457159;
// Declara funcao resolve
/////////////////////////
void resolve(int,int);

// Checa se podemos inserir num em quadrado
// Retorna 1 se ja ha um num igual na quadrado
// Retorna 0 se num nao esta na quadrado
///////////////////////////////////////////////
bool emquadr(int linha,int coluna,int num)
{
  int lin,col;
  coluna=(coluna/3)*3;
  linha=(linha/3)*3;
  for(col=0;col<3;col++)
    for(lin=0;lin<3;lin++)
      if(matriz[linha+lin][coluna+col]==num) return false;
  return true;
}

// Checa se podemos inserir num em coluna
// Retorna 1 se ja ha um num igual na coluna
// Retorna 0 se num nao esta na coluna
////////////////////////////////////////////
bool emcoluna(int coluna,int num)
{
  int linha;
  for(linha=0;linha<9;linha++)
      if(matriz[linha][coluna]==num) return false;
  return true;
}

// Checa se podemos inserir num em linha
// Retorna 1 se ja ha um num igual na linha
// Retorna 0 se num nao esta na linha
////////////////////////////////////////////
bool emlinha(int linha,int num)
{
  int coluna;
    for(coluna=0;coluna<9;coluna++)
      if(matriz[linha][coluna]==num) return false;
  return true;
}

// checa se podemos colocar o numero num
// em linha e coluna da ratriz
////////////////////////////////////////////
bool valido(int linha, int coluna, int num)
{
  return emlinha(linha,num)&&emcoluna(coluna,num)&&emquadr(linha,coluna,num);
}

// Funcao usada para imprimir o sudoku
///////////////////////////////////////
void imprimeMatriz()
{
  int linha,coluna;
  for(linha=0;linha<9;linha++)
  {
    for(coluna=0;coluna<9;coluna++) printf("%d ",matriz[linha][coluna]);
    printf("\nti");
  }
}

// Funcao usada para ler o sudoku do arquivo
// "sudoku.in"
////////////////////////////////////////////
void leMatrizArquivo(int mat[][9], int N){
	int i,j;
	//abre arquivo para leitura
	FILE *fp=fopen("sudoku.in", "r");
	for( i=0;i<N;i++){
		for(j=0;j<N;j++){
			fscanf(fp,"%d ",&mat[i][j]);
		}
	}
	//fecha o arquivo
	fclose(fp);
}

// Funcao principal do algoritmo (recrsiva)
// Iniciae gerencia a resolucao do sudoku
// recebe a celula atual
// iniciado na celula (0,0)
///////////////////////////////////////////
void resolve(int linha,int coluna)
{
  if(matriz[linha][coluna]!=0) 
	{// Se já tem um numero na celula recurte com a proxima celula
    if(coluna<8) resolve(linha,coluna+1);
    else 
      resolve(linha+1,0);
    passos++;
  } 
  else 
  {// se celula vazia
    int numero;
    for(numero=1;numero<=9;numero++)
    {// para cada numero de 1 a 9
      if(valido(linha,coluna,numero))
      {// insere o primeiro que couber e recurte com a proxima celula
        matriz[linha][coluna]=numero;
        if(coluna<8) resolve(linha,coluna+1);
  	    else 
    	    resolve(linha+1,0);
	      passos++;
      }
    }
    matriz[linha][coluna]=0;// reseta a celula se nao achar solucao, retorna
  }
	if(linha>8) imprimeMatriz();// chegou no final da matriz
}

int main()
{
  leMatrizArquivo(matriz,9);
  resolve(0,0);
  printf("passos: %d a:%d\n",passos,3*verif);
}
