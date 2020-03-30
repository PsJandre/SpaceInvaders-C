#include <allegro5/allegro.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#define LARGURA_TELA 512
#define ALTURA_TELA 512
#define FPS 60.0

ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_BITMAP *fundo = NULL;
ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
ALLEGRO_SAMPLE *somdetiro = NULL;
ALLEGRO_AUDIO_STREAM *musica = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_BITMAP *nave = NULL;
ALLEGRO_BITMAP *inimigo = NULL;
ALLEGRO_BITMAP *disparo = NULL;
ALLEGRO_BITMAP *disparo2 = NULL;
ALLEGRO_FONT *fonte = NULL;

int highscore;
int velInimigos= 20;
int numDeInimigos=36;
int pontuacao=0;
int nX=250;
int nY=450;
int tirosInimigos=3;
void t(void);
int ler();
void escrever(int p);
int bounding_box_collision();
void moveInimigos();
int TelaInicial();
int TelaGameOver();
int checarColisaoParede();
int checarColisaoChao();
int checarColisaoNave(struct tiros *t);
int colisao(int b1_x, int b1_y, int b1_w, int b1_h, int b2_x, int b2_y, int b2_w, int b2_h);
int checarColisoes(struct tiros* t);
struct tiros* criaTiro(int x, int y, int z);
struct inimigos* criaInimigos(int x, int y, int z);


struct tiros
{
    int posX;
    int posY;
    int ativar;
};
struct tiros* vetTiros[5];
struct tiros* vetTiros2[3];
struct inimigos
{
    int posX;
    int posY;
    int vivo;
};
struct inimigos* vetInimigos[36];

int iniciar()
{
    srand(time(NULL));
//inicia a biblioteca
    al_init();
//iniciar o timer com o intervalo que ele ira disparar
    timer = al_create_timer(1.0 / FPS);
//addon de audio
    al_install_audio();
//addon que da superote a exensoes de audio
    al_init_acodec_addon();
//cria um mixer de sons
    al_reserve_samples(2);
//cria um addon para uso de fontes
    al_init_font_addon();
    al_init_ttf_addon();

    fonte = al_load_font("OpenSans-Bold.ttf", 18, 0);

    //addon de keyboard
    al_install_keyboard();
    al_set_window_title(janela, "Invasores");
    janela=al_create_display(LARGURA_TELA, ALTURA_TELA);

    //fila de eventos
    fila_eventos=al_create_event_queue();
    al_register_event_source(fila_eventos, al_get_timer_event_source(timer));
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_register_event_source(fila_eventos, al_get_display_event_source(janela));

    return 0;

}

int main (void)
{
    //Variaveis
    int mov=1;
    int sair =0;
    int tX=0;
    int tY=0;
    int velTiro=5;
    int cont=0;
    int contFPS=0;
    int desenha=1;
    int numDeTiros=5;
    int vidas = 3;
    iniciar();
    highscore = ler();
    if(TelaInicial()==0)
        return 0;
    for(int i=0; i<6; i++)
    {
        for(int j=0; j<6; j++)
        {
            vetInimigos[cont]=criaInimigos(100+i*70,10+j*40,1);
            ++cont;
        }
    }
    for(int i=0; i<numDeTiros; i++)
    {
        vetTiros[i]=criaTiro(0,0,0);
    }
    for(int i=0; i<tirosInimigos; i++)
    {
        vetTiros2[i]=criaTiro(0,0,0);
    }
//carrega os samples
    somdetiro = al_load_sample("laser.wav");
//carrega o stream
    musica = al_load_audio_stream("A New Morning.ogg", 4, 1024);
//addon de imagem
    al_init_image_addon();
    //liga o stream no mixer
    al_attach_audio_stream_to_mixer(musica, al_get_default_mixer());
    //define que o stream vai tocar no modo repeat
    al_set_audio_stream_playmode(musica, ALLEGRO_PLAYMODE_LOOP);
    fundo = al_load_bitmap("FundoEspaco.png");
    al_draw_bitmap(fundo, 0, 0, 0);
    nave = al_load_bitmap("spaceship2.png");
    al_draw_bitmap(nave, 300, 300, 0);
    inimigo = al_load_bitmap("Enemy1.png");
    disparo = al_load_bitmap("tiro.png");
    disparo2 = al_load_bitmap("tiro2.png");
    al_flip_display();
    al_start_timer(timer);
    while(sair!=1)
    {
        sair=checarColisaoChao();
        if(numDeInimigos==0)
            sair=1;
        if(vidas<1)
            sair=1;

        ALLEGRO_EVENT evento;

        al_wait_for_event(fila_eventos, &evento);
        if(evento.type == ALLEGRO_EVENT_TIMER)
        {
            desenha=1;
        }

        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break;
        }

        if (evento.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            switch(evento.keyboard.keycode)
            {

            case ALLEGRO_KEY_UP:
                tX=nX+13;
                tY=nY+5;
                for(int i=0; i<numDeTiros; i++)
                {
                    if(vetTiros[i]->ativar==0)
                    {
                        al_play_sample(somdetiro, 1.0, 0.0,1.0,ALLEGRO_PLAYMODE_ONCE,NULL);
                        vetTiros[i]->posX=tX;
                        vetTiros[i]->posY=tY;
                        vetTiros[i]->ativar=1;
                        break;
                    }
                }

                break;

            case ALLEGRO_KEY_LEFT:
                if(nX-10>0)
                    nX-=10;
                break;
            case ALLEGRO_KEY_RIGHT:
                if(nX+10<492)
                    nX+=10;
                break;

            case ALLEGRO_KEY_ESCAPE:
                sair = 1;
            }
        }


        if(contFPS==120)
        {

            if(mov==1)
            {
                if(checarColisaoParede()==1)
                    velInimigos*=-1;
            }
            moveInimigos(mov);
            mov*=-1;

        }

        if(al_is_event_queue_empty(fila_eventos)&&desenha)
        {
            t();
            al_clear_to_color(al_map_rgb(255, 255, 255));
            al_draw_bitmap(fundo, 0, 0, 0);

            al_draw_textf(fonte, al_map_rgb(255, 255, 255), 10, 480, 0, "Pontos: %d Vidas: %d", pontuacao, vidas);


            for(int i=0; i<36; i++)
            {
                if(vetInimigos[i]->vivo==1)
                    al_draw_bitmap(inimigo, vetInimigos[i]->posX, vetInimigos[i]->posY, 0);

            }
            if(contFPS==120) contFPS=0;
            for (int i=0; i<tirosInimigos; i++)
            {
                if(vetTiros2[i]->ativar==1)
                {
                    vetTiros2[i]->posY+=velTiro;
                    al_draw_bitmap(disparo2, vetTiros2[i]->posX, vetTiros2[i]->posY, 0);
                    if(checarColisaoNave(vetTiros2[i]))
                    {
                        --vidas;
                        vetTiros2[i]->ativar=0;
                    }
                    if(vetTiros2[i]->posY>500)
                        vetTiros2[i]->ativar=0;
                }

            }
            for(int i=0; i<numDeTiros; i++)
            {
                if(vetTiros[i]->ativar==1)
                {
                    vetTiros[i]->posY-=velTiro;
                    al_draw_bitmap(disparo, vetTiros[i]->posX, vetTiros[i]->posY, 0);
                    if(checarColisoes(vetTiros[i]))
                    {
                        vetTiros[i]->ativar=0;

                    }
                }
                if(vetTiros[i]->posY<0)
                    vetTiros[i]->ativar=0;
            }
            al_draw_bitmap(nave, nX, nY, 0);
            desenha=0;
            ++contFPS;
            al_flip_display();
        }
    }
    al_destroy_audio_stream(musica);
    al_destroy_sample(somdetiro);
    TelaGameOver();
    return 0;
}
int ler (){
int p;
int numeros[3];
FILE *i;
char x[3];
i = fopen("pontuacao", "r");
fgets(x, sizeof(x+1), i);

numeros[0]=x[0]-'0';
numeros[1]=x[1]-'0';
numeros[2]=x[2]-'0';
p=numeros[0]*100+numeros[1]*10+numeros[2];
fclose(i);
return  p;
}

void escrever(int p){
FILE *i;
i = fopen("pontuacao", "w");
char o[3];
o[0]=p/100+48;
o[1]=((p/10)%10)+48;
o[2]=p%10+48;
printf("%s", &o);
fprintf(i,"%s", o);


fclose(i);
}
void t(void){
for(int i=0; i<36; i++)
        {
            if(vetInimigos[i]->vivo==1)
            {
                int aleatorio = rand()%100;
                if(aleatorio<1)
                {
                    for(int j=0; j<tirosInimigos; j++)
                    {
                        if(vetTiros2[j]->ativar==0)
                        {
                            vetTiros2[j]->posX=vetInimigos[i]->posX+8;
                            vetTiros2[j]->posY=vetInimigos[i]->posY;
                            vetTiros2[j]->ativar=1;
                           break;
                        }
                    }
                }

            }
        }

}

int colisao(int b1_x, int b1_y, int b1_w, int b1_h, int b2_x, int b2_y, int b2_w, int b2_h)
{
    if ((b1_x > b2_x + b2_w - 1) || // is b1 on the right side of b2?
            (b1_y > b2_y + b2_h - 1) || // is b1 under b2?
            (b2_x > b1_x + b1_w - 1) || // is b2 on the right side of b1?
            (b2_y > b1_y + b1_h - 1))   // is b2 under b1?
    {
        // no collision
        return 0;
    }

    // collision
    return 1;
}
int checarColisaoNave(struct tiros *t)
{
    for(int i=0; i<tirosInimigos; i++)
    {
        if(vetTiros2[i]->ativar)
        {
            if(colisao(nX,nY,30,30,t->posX, t->posY, 5, 20))
                return 1;
        }
    }
    return 0;
}
int checarColisoes(struct tiros* t)
{
    for(int i=0; i<36; i++)
    {
        if(colisao(vetInimigos[i]->posX, vetInimigos[i]->posY, 30, 22, t->posX, t->posY,5, 20))
        {
            vetInimigos[i]->vivo=0;
            vetInimigos[i]->posX=0;
            vetInimigos[i]->posY=0;
            --numDeInimigos;
            pontuacao+=10;
            return 1;
        }
    }
    return 0;
}

int checarColisaoParede()
{
    for(int i=0; i<36; i++)
    {
        if(vetInimigos[i]->vivo==1)
        {
            if(vetInimigos[i]->posX<=0||vetInimigos[i]->posX>=465)
            {
                return 1;
            }

        }
    }
    return 0;
}

int checarColisaoChao()
{
    for(int i=0; i<36; i++)
    {
        if(vetInimigos[i]->vivo==1)
        {
            if(vetInimigos[i]->posY>=440)
            {
                return 1;
            }

        }
    }
    return 0;
}

void moveInimigos(int mov)
{
    for(int i=0; i<36; i++)
    {
        if(vetInimigos[i]->vivo==1)
        {


            if(mov==1)
            {
                vetInimigos[i]->posX+=velInimigos;

            }
            else
            {
                vetInimigos[i]->posY+=20;

            }


        }
    }

}

struct inimigos* criaInimigos(int x, int y, int z)
{
    struct inimigos *I;
    I=malloc(sizeof(struct inimigos));
    I->posX = x;
    I->posY = y;
    I->vivo = z;
    return I;
};

struct tiros* criaTiro(int x, int y, int z)
{
    struct tiros *T;
    T=malloc(sizeof (struct tiros));
    T->posX = x;
    T->posY = y;
    T->ativar=z;
    return T;
}

int TelaInicial()
{
    musica = al_load_audio_stream("musicaMenu.ogg", 4, 1024);
    al_init_image_addon();
    //liga o stream no mixer
    al_attach_audio_stream_to_mixer(musica, al_get_default_mixer());
    //define que o stream vai tocar no modo repeat
    al_set_audio_stream_playmode(musica, ALLEGRO_PLAYMODE_LOOP);
    fundo = al_load_bitmap("TelaDeMenu.png");
    al_draw_bitmap(fundo, 0, 0, 0);
    al_flip_display();

    while(1)
    {
        ALLEGRO_EVENT evento;

        al_wait_for_event(fila_eventos, &evento);
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            return 0;
        }
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN)
        {

            switch(evento.keyboard.keycode)
            {

            case ALLEGRO_KEY_ESCAPE:
                return 0;

            case ALLEGRO_KEY_ENTER:
                al_destroy_audio_stream(musica);


                return 1;
                al_draw_bitmap(fundo, 0, 0, 0);
                al_flip_display();

            }
        }
    }
}


int TelaGameOver()
{
    if(highscore<pontuacao){
    escrever(pontuacao);
    highscore=pontuacao;
    }
    musica = al_load_audio_stream("MusicaGameOver.ogg", 4, 1024);
    al_init_image_addon();
//liga o stream no mixer
    al_attach_audio_stream_to_mixer(musica, al_get_default_mixer());
//define que o stream vai tocar no modo repeat
    al_set_audio_stream_playmode(musica, ALLEGRO_PLAYMODE_LOOP);
    fundo = al_load_bitmap("GameOver.png");
    al_draw_bitmap(fundo, 0, 0, 0);
    al_flip_display();
    while(1)
    {
        ALLEGRO_EVENT evento;
        al_wait_for_event(fila_eventos, &evento);
        if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            return 0;
        }
        if (evento.type == ALLEGRO_EVENT_KEY_DOWN)
        {

            switch(evento.keyboard.keycode)
            {

            case ALLEGRO_KEY_ESCAPE:
                return 1;



            }
        }

        al_draw_bitmap(fundo, 0, 0, 0);
        al_draw_textf(fonte, al_map_rgb(0, 0, 0), 200, 480, 0, "Pontos: %d ", pontuacao);
        al_draw_textf(fonte, al_map_rgb(0, 0, 0), 140, 430, 0, "Pontuação maxima: %d ", highscore);
        al_flip_display();
    }
    al_destroy_audio_stream(musica);
    al_destroy_display(janela);
    al_destroy_event_queue(fila_eventos);

}





