#define OLC_PGE_APPLICATION
#include <stdio.h>
#include "olc.h"
#include "data.cpp"
#define height 50
#define width 80
#define waittime 20

struct map{
	unsigned char c;
	short fg;
	short bg;
	bool operator==(const map&p)const{return( ((c==p.c)&&(fg==p.fg)&&(bg==p.bg)));};
	bool operator!=(const map&p)const{return(!((c==p.c)&&(fg==p.fg)&&(bg==p.bg)));};
};

char font[1<<14];
char key; // actually a boolfield
map newmap[height*width],oldmap[height*width]; // screen buffers



int mod(int a,int b){a%=b;return(a>=0?a:b+a);} // sigh . . .
int min(int a,int b){return(a>b?b:a);}
int max(int a,int b){return(a<b?b:a);}

class Game : public olc::PixelGameEngine{public:
	Game(){sAppName="Game";}
	bool OnUserCreate()override{
		// initialise newmap
		clear();
		init();
		render();
		banner();
		return(true);
	}

	void clear(){for(int i=0;i<height*width;i++)newmap[i]={0,0x0fff,0x0000};}

	olc::Pixel sga(short color){ // transforms color from 8bit sga space to 24bit truecolor (strictly speaking its 12bit but whatever)
		//t a = (color&0xf000)<< 0; // wont be using it anyway
		int r = (color&0x0f00)<< 4;
		int g = (color&0x00f0)<< 8;
		int b = (color&0x000f)<<12;

		r=r|r>>4|r>>8|r>>12;
		g=g|g>>4|g>>8|g>>12;
		b=b|b>>4|b>>8|b>>12;

		return olc::Pixel(r,g,b);
	}

	void pc(char c, int sx, int sy, short fgc, short bgc){
		olc::Pixel fg = sga(fgc);
		olc::Pixel bg = sga(bgc);

		for(int x=0;x<8;x++)
		for(int y=0;y<8;y++)
		Draw(sy*8+y,sx*8+x,((char)font[(unsigned char)c*8+x]&(char)(128>>y))?fg:bg);

		return;
	}
	void ps(const char*c,int l,int sx,int sy,short int fgc,short int bgc){for(int i=0;i<l;i++)newmap[sx*width+sy+i]=(map){(unsigned char)c[i],fgc,bgc};}
	void pi(int c,int sx,int sy,short int fgc,short int bgc){
		// XXX it still ignores the 0s . . .
		char a[10],b=0;
		for(;c>0;c/=10)a[b++]=(c%10)+48;
		for(;b>0;b--)newmap[sx*width+sy+b]=(map){(unsigned char)a[b-1],fgc,bgc};
	}

	void refresh(){
		for(int x=0;x<height;x++)for(int y=0;y<width;y++)
			if(newmap[x*width+y]!=oldmap[x*width+y]){
				pc(newmap[x*width+y].c,x,y,newmap[x*width+y].fg,newmap[x*width+y].bg);
				oldmap[x*width+y]=newmap[x*width+y];
			}
	}

	void banner(){
		// 100x7, 71x7
		char banner1[]="      ____________________________________________________________________________________________       /   ______       _                        __         __  __         ______                  /_     /   / __/ /____ _(_)____    _____ ___ __  / /____    / /_/ /  ___   / __/ /____ ________    / /    /   _\\ \\/ __/ _ `/ / __/ |/|/ / _ `/ // / / __/ _ \\  / __/ _ \\/ -_) _\\ \\/ __/ _ `/ __(_-<   / /    /   /___/\\__/\\_,_/_/_/  |__,__/\\_,_/\\_, /  \\__/\\___/  \\__/_//_/\\__/ /___/\\__/\\_,_/_/ /___/  / /    /___________________________________/___/___________________________________________________/ /     \\___________________________________\\___\\/___________________________________________________/     ";
		char banner2[]=" _____________________________________________________________________ |                                                                     ||     __                                              __              ||    (_ _|_ _.o._     _.      _|_ _     _|_|_  _     (_ _|_ _.._ _    ||    __) |_(_||| \\/\\/(_|\\/     |_(_)     |_| |(/_    __) |_(_|| _>    ||                       /                                             ||_____________________________________________________________________|";
		for(int x=0;x<71;x++)
		for(int y=0;y<7;y++)
		newmap[(10+y)*width+4+x]=(map){(unsigned char)banner2[x+y*71],0x08bf,0x0000};
		//pc(banner2[x+y*71],10+y,4+x,0x08bf,0x0000);
		refresh();
	}

	void legal(){
		clear();
		for(int i=0,x=0,y=5;i++<1627;disclaimer[i]==10?(x=0,y++):(newmap[y*width+x++]=(map){disclaimer[i],0x0fff,0x0000},0));
		refresh();
	}

	void render(){
		/*
		clear map
		for all squares in an area around player
		 if square negative or outside of map boundaries
		  draw shadow
		 else
		  draw tile from map
		  for all items
		   if item in square
		    draw item
		  for all npcs
		   if npc in square
		    draw npc
		draw player
		*/

		clear();

		// XXX rewrite this shit
		for(int x=-1*viewrad;x<viewrad;x++)
		for(int y=-1*viewrad;y<viewrad;y++)
		if(x*x+y*y<viewrad*viewrad){
			int lx=x+player.x,ly=y+player.y;
			// XXX run line of sight here
			if(lx<0 || ly<0 || lx+1>mapsize || ly>mapsize){ // if outside of map
				newmap[(height/2+x)*width+width/4+y]=(map){'#',0x0666,0x0000};
			}else{
				newmap[(height/2+x)*width+width/4+y]=(map){floors[current_floor].lvl[lx*mapsize+ly],0x0ccc,0x0000};
				if(floors[current_floor].nc>0){
					for(int i=0;i<floors[current_floor].nc;i++){
						if(lx==floors[current_floor].npcs[i].x && ly==floors[current_floor].npcs[i].y && floors[current_floor].npcs[i].hp>0){
							newmap[(height/2+x)*width+width/4+y]={'g',0x08f8,0x0000};
						}
					}
				}

				if(floors[current_floor].ic>0)
				for(int i=0;i<floors[current_floor].ic;i++)
					if(lx==floors[current_floor].items[i].x && ly==floors[current_floor].items[i].y && floors[current_floor].items[i].type!=Item::BLANK){
						unsigned char c=0x27;
						switch(floors[current_floor].items[i].type){
							case Item::WEAPON:c=0x7c;break;
							case Item::ARMOR:c=0x9d;break;
							case Item::FOOD:c=0x07;break;
							case Item::RING:c=0x09;break;
							case Item::POTION:c=0xa8;break;
						}
						newmap[(height/2+x)*width+width/4+y]={c,0x0df6,0x0000}; // XXX update for item type
					}
			}
		}

		newmap[(height/2)*width+width/4]={'@',0x0fff,0x0000};

		// health
		newmap[0]=(map){'|',0x0ff8,0x0000};
		newmap[17]=(map){'|',0x0ff8,0x0000};
		for(int i=0;i<16;i++)newmap[i+1]=(player.hp>i*16)?(map){0xe9,0x04f0,0x0440}:(map){'-',0x0f44,0x0400};

		// XXX for the following two, should we leave it or should we use pi?
		// level
		ps("LVL:",4,1,0,0x0fff,0x0000);
		if(player.lvl>99)newmap[width+4]=(map){(unsigned char)(player.lvl/100+48),0x0fff,0x0000};
		if(player.lvl>9)newmap[width+5]=(map){(unsigned char)((player.lvl/10)%10+48),0x0fff,0x0000};
		newmap[width+6]=(map){(unsigned char)(player.lvl%10+48),0x0fff,0x0000};

		// floor
		ps("Floor:",6,2,0,0x0fff,0x0000);
		if(current_floor>99)newmap[2*width+6]=(map){(unsigned char)(current_floor/100+48),0x0fff,0x0000};
		if(current_floor>9)newmap[2*width+7]=(map){(unsigned char)((current_floor/10)%10+48),0x0fff,0x0000};
		newmap[2*width+8]=(map){(unsigned char)(current_floor%10+48),0x0fff,0x0000};

		refresh();
	}

	void inventory(){
		// XXX border
		// also offset two further in
		// 5+2*i ?
		// maybe use a separate var to keep track of how far in we are?

		for(int i=0;i<player.items;i++){
			switch(player.inven[i].type){
				case Item::WEAPON:
					newmap[(5+i)*width+3]=(map){0x7c,0x0fff,0x0000};
					ps("Sword ",6,5+i,5,0x0fff,0x0000);
					pi(player.inven[i].stat,5+i,10,0x0fff,0x0000);
					newmap[(5+i)*width+13]=(map){'(',0x0fff,0x0000};
					pi(player.inven[i].durability,5+i,13,0x0fff,0x0000);
					newmap[(5+i)*width+17]=(map){')',0x0fff,0x0000};
				break;case Item::ARMOR:
					newmap[(5+i)*width+3]=(map){0x9d,0x0fff,0x0000};
					ps("Armor ",6,5+i,5,0x0fff,0x0000);
					pi(player.inven[i].stat,5+i,10,0x0fff,0x0000);
					newmap[(5+i)*width+13]=(map){'(',0x0fff,0x0000};
					pi(player.inven[i].durability,5+i,13,0x0fff,0x0000);
					newmap[(5+i)*width+17]=(map){')',0x0fff,0x0000};
				break;case Item::FOOD:
					newmap[(5+i)*width+3]=(map){0x07,0x0fff,0x0000};
					ps("Food ",5,5+i,5,0x0fff,0x0000);
					pi(player.inven[i].stat,5+i,11,0x0fff,0x0000);
					newmap[(5+i)*width+13]=(map){'(',0x0fff,0x0000};
					pi(player.inven[i].durability,5+i,13,0x0fff,0x0000);
					newmap[(5+i)*width+17]=(map){')',0x0fff,0x0000};
				break;case Item::RING:
					newmap[(5+i)*width+3]=(map){0x09,0x0fff,0x0000}; // 0xf7
					ps("Ring of ",8,5+i,5,0x0fff,0x0000);
					switch(player.inven[i].stat&0xc0){
						case 0:ps("Health    ",10,5+i,13,0x0fff,0x0000);break;
						case 1:ps("Strength  ",10,5+i,13,0x0fff,0x0000);break;
						case 2:ps("Dexterity ",10,5+i,13,0x0fff,0x0000);break;
						case 3:ps("Longevity ",10,5+i,13,0x0fff,0x0000);break;
						default:break;
					}
					/*
					pi(player.inven[i].stat&0x3f,23,5+i,0x0fff,0x0000);
					pc('(',26,5+i,0x0fff,0x0000);
					pi(player.inven[i].durability,27,5+i,0x0fff,0x0000);
					pc(')',30,5+i,0x0fff,0x0000);
					*/
				break;case Item::POTION:
					newmap[(5+i)*width+3]=(map){0xa8,0x0fff,0x0000}; // 0xab 0xa8 0xe5 0xeb
					ps("Potion of ",10,5+i,5,0x0fff,0x0000);
					switch(player.inven[i].stat&0xc0){
						case 0:ps("Health    ",10,5+i,15,0x0fff,0x0000);break;
						case 1:ps("Strength  ",10,5+i,15,0x0fff,0x0000);break;
						case 2:ps("Dexterity ",10,5+i,15,0x0fff,0x0000);break;
						case 3:ps("Longevity ",10,5+i,15,0x0fff,0x0000);break;
						default:break;
					}
					/*
					pi(player.inven[i].stat&0x3f,23,5+i,0x0fff,0x0000);
					pc('(',26,5+i,0x0fff,0x0000);
					pi(player.inven[i].durability,27,5+i,0x0fff,0x0000);
					pc(')',30,5+i,0x0fff,0x0000);
					*/
				break;default:break;
			}
		}

		refresh();

	}

	void gethelp(){ // XXX
		ps("Stairway to the Stars",21,5,5,0x0bbb,0x0000);
		ps("Kill enemies, collect loot, climb the tower",43,6,5,0x0bbb,0x0000);
		ps("WASD - movement (move into enemies to attack)",45,7,5,0x0bbb,0x0000);
		ps("E - pick up loot, climb stairs",30,8,5,0x0bbb,0x0000);
		ps("I - open inventory",18,9,5,0x0bbb,0x0000);
		ps("L - view OLC license",20,10,5,0x0bbb,0x0000);
		ps("Q - exit game",13,11,5,0x0bbb,0x0000);
		refresh();
	}

	bool OnUserUpdate(float fElapsedTime)override{
		key=0;
		// XXX maybe make the following use DEFINEd keys?
		// maybe define them in a string
		// that miiight make it harder to match against olc tho
		if(GetKey(olc::W).bPressed)key=1;
		if(GetKey(olc::A).bPressed)key=2;
		if(GetKey(olc::S).bPressed)key=3;
		if(GetKey(olc::D).bPressed)key=4;
		if(GetKey(olc::E).bPressed)key=5; // pick up item, wait a turn, etc
		if(GetKey(olc::I).bPressed)inventory();
		if(GetKey(olc::U).bPressed)key=6; // use item in inventory
		if(GetKey(olc::Q).bPressed)exit(0); // XXX smth?
		if(GetKey(olc::H).bPressed)gethelp(); // '?' nonexistent in olc
		if(GetKey(olc::L).bPressed)legal();
		if(GetKey(olc::G).bPressed){for(int i=0;i<waittime;i++)step(0);render();} // wait a while
		if(GetKey(olc::O).bPressed){printf("floor:%i\nhp:%i\nlvl:%i\ninven size:%i\n\n",current_floor,player.hp,player.lvl,player.items);
			clear();
			for(int x=0;x<16;x++)for(int y=0;y<16;y++)
			newmap[(x<<1)*width+(y<<1)]=(map){(unsigned char)((x<<4)+y),0x0fff,0x0000};refresh(); // XXX colors?
		}

		if(key){
			step(key);
			render();
		}

		return true;
	}
};

int main(){
	// import font
	FILE*f=fopen("font","r");
	for(int i=0;i<3200;i++)*(font+i)=getc(f);
	fclose(f);

	// olcpge main loop
	Game game;if(game.Construct(width*8, height*8, 2, 2))game.Start();return(0);
}

/*
los

npc ai

display data
 inventory
 stats

fix current main weapon/shield use

*/
