#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* n de chars possiveis levados em conta */
#define CHARS_N  257

/* Marcador de final de arquivo */
#define FIM_CHAR    256

/* tamanho dos chars em bits */
#define CHAR_SIZE   8

/* struct da arvore de Hufman */
typedef struct htree
{
    struct htree *esqerda;
    struct htree *direita;
    int letra;
    int freq;
} htree;

///////////////////////////////////
// Funcoes
///////////////////////////////////
int comparaHTree(const void *a, const void *b);
htree *MontaHTree(int frequencias[]);
char *Concat(char *prefix, char letra);
void Error(const char *msg);
void CortaArvore(htree *arvore);
void PercorreArvore(htree *arvore, char **tabela, char *prefix);
char **MontaTabela(int frequencias[]);
void ApagaTabela(char *tabela[]);
void EscreveHeader(FILE *out, int frequencias[]);
int *LeHeader(FILE *in);
void BitWriter(const char *charCod, FILE *out);
int ReadBit(FILE *in);
int DecChar(FILE *in, htree *arvore);
void Dec(FILE *in, FILE *out);
void Cod(FILE *in, FILE *out);


/////////////////////////////////
// Implementacoes
/////////////////////////////////


/* Comparador de arvores */
int comparaHTree(const void *a, const void *b)
{
    const htree **x = (const htree **) a, **y = (const htree **) b;
    if((*x)->freq == (*y)->freq) return 0;
    else return ((*x)->freq < (*y)->freq) ? 1 : -1;
}

/* Construtor de arvore, recebe tabela de frequencia */
htree *MontaHTree(int frequencias[])
{
    int i, len = 0;
    htree *fila[CHARS_N];

    /* Cria arvores para cada char, adicionando a fila */
    for(i = 0; i < CHARS_N; i++)
    {
        if(frequencias[i])
        {
            htree *tempTree = (htree *)calloc(1, sizeof(htree));
            tempTree->letra = i;
            tempTree->freq = frequencias[i];

            fila[len++] = tempTree;
        }
    }

    while(len > 1)
    {
        htree *tempTree = (htree *)malloc(sizeof(htree));

        /* organiza por ordem de frequencia (menos frequentes por ultimo) */
        qsort(fila, len, sizeof(htree *), comparaHTree);

        /* tira da fila as 2 arvores de menor frequencia, e faz nova arvore */
				/* com as mesmas */
        tempTree->esqerda = fila[--len];
        tempTree->direita = fila[--len];
        tempTree->freq = tempTree->esqerda->freq + tempTree->direita->freq;

				/* insere de volta para a fila */
        fila[len++] = tempTree;
    }

		/* Ultima arvore na fila e a arvore completa */
    return fila[0];
}

/* Concatenador se palavras e letras */
char *Concat(char *prefix, char letra)
{
    char *result = (char *)malloc(strlen(prefix) + 2);
    sprintf(result, "%s%c", prefix, letra);
    return result;
}

/* Handle de erros */
void Error(const char *msg)
{
    fprintf(stderr, "ERRO: %s\n", msg);
    exit(1);
}

/* recursivamente desaloca arvore de memoria */
void CortaArvore(htree *arvore)
{
    if(arvore)
    {
        CortaArvore(arvore->esqerda);
        CortaArvore(arvore->direita);
        free(arvore);
    }
}

/* percorre arvore recirsivamente montando tabela de codificacao */
void PercorreArvore(htree *arvore, char **tabela, char *prefix)
{
    if(!arvore->esqerda && !arvore->direita) tabela[arvore->letra] = prefix;
    else
    {
        if(arvore->esqerda)
				    PercorreArvore(arvore->esqerda, tabela, Concat(prefix, '0'));
        if(arvore-> direita)
				    PercorreArvore(arvore->direita, tabela, Concat(prefix, '1'));
        free(prefix);
    }
}

/* monta tabela de codificacao com base na tabela de frequencia */
char **MontaTabela(int frequencias[])
{
    static char *tabela[CHARS_N];
    char *prefix = (char *)calloc(1, sizeof(char));
    htree *arvore = MontaHTree(frequencias);
    PercorreArvore(arvore, tabela, prefix);
    CortaArvore(arvore);

    return tabela;
}

/* desaloca tabela de codificacao de memoria */
void ApagaTabela(char *tabela[])
{
    int i;
    for(i = 0; i < CHARS_N; i++) if(tabela[i]) free(tabela[i]);
}

/* Constroi o cabecalhio do arquivo com base nas frequencias */
void EscreveHeader(FILE *out, int frequencias[])
{
    int i, count = 0;

    for(i = 0; i < CHARS_N; i++) if(frequencias[i]) count++;
    fprintf(out, "%d\n", count);

    for(i = 0; i < CHARS_N; i++)
        if(frequencias[i]) fprintf(out, "%d %d\n", i, frequencias[i]);
}

/* le cabecalhio de um arquivo codificado e monta tabela de frequencias*/
int *LeHeader(FILE *in)
{
    static int frequencias[CHARS_N];
    int i, count, letra, freq;

    if(fscanf(in, "%d", &count) != 1) Error("Arquivo de entrada invalido.");

    for(i = 0; i < count; i++)
    {
        if((fscanf(in, "%d %d", &letra, &freq) != 2)
           || letra < 0 || letra >= CHARS_N)
					     Error("Arquivo de entrada invalido.");

        frequencias[letra] = freq;
    }
		/* discarta ultima linha */
    fgetc(in);

    return frequencias;
}

/* escreve bit para arquivo */
void BitWriter(const char *charCod, FILE *out)
{
    /* buffer de bits e contagem dos mesmos */
    static int bits = 0, bitcount = 0;

    while(*charCod)
    {
        /* bufferiza bits da esquerda */
        bits = bits * 2 + *charCod - '0';
        bitcount++;

        /* quando buffer atingir tamanho de char, escreve o mesmo e reseta */
				/* contagem/buffer */
        if(bitcount == CHAR_SIZE)
        {
            fputc(bits, out);
            bits = 0;
            bitcount = 0;
        }

        charCod++;
    }
}

/* le unico bit de arquivo input */
int ReadBit(FILE *in)
{
    /* buffer de bits e contagem dos mesmos */
    static int bits = 0, bitcount = 0;
    int nextbit;

    if(bitcount == 0)
    {
        bits = fgetc(in);
        bitcount = (1 << (CHAR_SIZE - 1));
    }

    nextbit = bits / bitcount;
    bits %= bitcount;
    bitcount /= 2;

    return nextbit;
}

 /* decodifica e retorna char a partir de IN e arvore H */
int DecChar(FILE *in, htree *arvore)
{
    while(arvore->esqerda || arvore->direita)
    {
        if(ReadBit(in)) arvore = arvore->direita;
        else arvore = arvore->esqerda;

        if(!arvore) Error("Arquivo de entrada invalido.");
    }
    return arvore->letra;
}

/* decodifica arquivo codificado e salva */
void Dec(FILE *in, FILE *out)
{
    int *frequencias, c;
    htree *arvore;

    frequencias = LeHeader(in);
    arvore = MontaHTree(frequencias);

    while((c = DecChar(in, arvore)) != FIM_CHAR)
        fputc(c, out);

    CortaArvore(arvore);
}

/* codifica arquivo deodificado e salva */
void Cod(FILE *in, FILE *out)
{
    int c, frequencias[CHARS_N] = { 0 };
    char **tabela;

    while((c = fgetc(in)) != EOF) frequencias[c]++;

    frequencias[FIM_CHAR] = 1;
    rewind(in);

    tabela = MontaTabela(frequencias);
    EscreveHeader(out, frequencias);

    while((c = fgetc(in)) != EOF)
        BitWriter(tabela[c], out);

    /* escreve char sinalizador de fim de arquivo */
    BitWriter(tabela[FIM_CHAR], out);

    /* flush oputput buffer */
    BitWriter("0000000", out);

    ApagaTabela(tabela);
}

int main(int argc, char *argv[])
{
    FILE *in, *out;

    if(argc != 4 || (strcmp(argv[1], "-c") && strcmp(argv[1], "-d")))
    {
        fprintf(stderr, "Uso: %s [-c,-d] input output\n", argv[0]);
        exit(0);
    }

    if(!(in = fopen(argv[2], "rb")))
        Error("Nao e possivel abrir arquivo de entrada.");
    else if((out = fopen(argv[3], "rb")))
        Error("Arquivo de saida ja existe.");
    else if(!(out = fopen(argv[3], "wb")))
        Error("Arquivo de saida nao pode ser aberto.");

    if(!strcmp(argv[1], "-c")) Cod(in, out);
    else Dec(in, out);

    fclose(in);
    fclose(out);

    return 0;
}
