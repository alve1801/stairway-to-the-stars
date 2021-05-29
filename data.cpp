#define mapsize 30
#define roomcount 5
// XXX the following can bug the game
#define minroomsize 3
#define maxroomsize 9
#define npccount 3
#define itemcount 3
#define viewrad 7
#define max_inven 10
#define endor_floor 10 // endgame

struct Item{
	int x,y;
	enum{WEAPON,ARMOR,FOOD,RING,POTION,BLANK}type;
	unsigned char durability;
	unsigned char stat;
};

struct Entity{
	int x,y;
	unsigned char lvl,hp,items; // XXX xp? gold?
	unsigned char str,dex,con,hunger,thirst,fatigue; // unused for now
	Item*inven,*weapon,*armor,*ring[3]; // XXX where tf do we store the sizes?
};

struct Floor{
	unsigned char lvl[mapsize*mapsize];
	// entry and exit
	struct stairs {
		int x,y;
	} in,out;
	int ic,nc;
	Item*items;
	Entity*npcs;
};

unsigned int r=0,a=1103515245,c=12345;
unsigned int rando(){return(r=a*r+c);} // fuck stdlib

// no seriously, cant we UNdefine functions or smth?
//#undef rand - doesnt work

unsigned char current_floor=0,farthest_floor=0;
Floor floors[endor_floor];
Entity player;

// XXX if the following are only used in genlvl, why are they here?
struct{
  int x;
  int y;
} rooms[roomcount];
int rx,rxm,ry,rym;

bool isfree(int x,int y,int lvl=current_floor){
	return floors[lvl].lvl[x*mapsize+y]!='#';
}

void genlvl(int rin){
	// generate map
	//  initialise room counter
	//  if not enough rooms
	//  select upper left corner of room
	//  select size of room
	//  (check if all correct?)
	//  decrease room counter, draw room in map
	//  calculate center of room
	//  draw line from center of room to previous center
	//  swap centers, repeat
	// put player in first room, exit in last

	// initialize r here if you want a specific floor layout

	// following is from previous rough project
	for(int i=0;i<mapsize*mapsize;i++)floors[rin].lvl[i]='#'; // memset
	for(int i=0;i<roomcount;i++){ // intialise rooms
		// x coords
		rx=rando()%(mapsize-maxroomsize)+1; // upper left corner
		rxm=rando()%(maxroomsize-minroomsize)+minroomsize; // upper right corner
		rooms[i].x=rx+rxm/2; // center of room on x
		// y coords
		ry=rando()%(mapsize-maxroomsize)+1; // lower left corner
		rym=rando()%(maxroomsize-minroomsize)+minroomsize; // lower right corner
		rooms[i].y=ry+rym/2; // center of room on y
		// put room in map
		for(int x=rx;x<rx+rxm;x++)
		for(int y=ry;y<ry+rym;y++)
		floors[rin].lvl[x*mapsize+y]=' ';
	}

	for(int i=0;i<(roomcount-1);i++){ // initialise corridors
		for(int x=rooms[i].x;x!=rooms[i+1].x;(rooms[i].x>rooms[i+1].x)?x--:x++)if(floors[rin].lvl[x*mapsize+rooms[i].y]=='#')floors[rin].lvl[x*mapsize+rooms[i].y]='.'; // horizontal part
		for(int y=rooms[i].y;y!=rooms[i+1].y;(rooms[i].y>rooms[i+1].y)?y--:y++)if(floors[rin].lvl[rooms[i+1].x*mapsize+y]=='#')floors[rin].lvl[rooms[i+1].x*mapsize+y]='.'; // vertical part
	}

	floors[rin].lvl[rooms[0].x*mapsize+rooms[0].y]='v';
	floors[rin].in={rooms[0].x,rooms[0].y};
	for(int i=1;i<roomcount-1;i++)floors[rin].lvl[rooms[i].x*mapsize+rooms[i].y]='o';
	floors[rin].lvl[rooms[roomcount-1].x*mapsize+rooms[roomcount-1].y]='^';
	floors[rin].out={rooms[roomcount-1].x,rooms[roomcount-1].y};

	//player.x=rooms[0].x;
	//player.y=rooms[0].y;

	// generate items
	//  initialise item counter
	//  if not enough items
	//   select coordinates at random
	//   check if in room
	//    if not select new
	//    if yes pick stats, initialise item, decrease item counter, select new
	// foreach item
	//  check db if item taken
	//  if yes, drop item

	// XXX finetune stats

	floors[rin].ic=itemcount;
	floors[rin].items=(Item*)malloc(itemcount*sizeof(Item)); // good!
	for(int i=0;i<itemcount;i++){
		itemloc:
		floors[rin].items[i].x=rando()%mapsize;
		floors[rin].items[i].y=rando()%mapsize;
		if(floors[rin].lvl[floors[rin].items[i].x*mapsize+floors[rin].items[i].y]=='#')
			goto itemloc;
		//items[i].type=Item::WEAPON;
		switch(rando()%5){
			case 0:floors[rin].items[i].type=Item::WEAPON;break;
			case 1:floors[rin].items[i].type=Item::ARMOR;break;
			case 2:floors[rin].items[i].type=Item::FOOD;break;
			case 3:floors[rin].items[i].type=Item::RING;break;
			case 4:floors[rin].items[i].type=Item::POTION;break;
		}
		floors[rin].items[i].durability=100;
		floors[rin].items[i].stat=10;
	}

	// generate npcs
	//  basically same as items
	floors[rin].nc=npccount;
	floors[rin].npcs=(Entity*)malloc(npccount*sizeof(Entity));
	for(int i=0;i<npccount;i++){
		npcloc:
		floors[rin].npcs[i].x=rando()%mapsize;
		floors[rin].npcs[i].y=rando()%mapsize;
		if(floors[rin].lvl[floors[rin].npcs[i].x*mapsize+floors[rin].npcs[i].y]=='#')
			goto npcloc;
		floors[rin].npcs[i].lvl=rando()%player.lvl;
		floors[rin].npcs[i].hp=5;
		floors[rin].npcs[i].items=0;
		//for(int i=0;i<10;i++)npcs[i].inven[i]=nullptr;
		floors[rin].npcs[i].ring[0]=floors[rin].npcs[i].ring[1]=floors[rin].npcs[i].ring[2]=nullptr;
	}
}



void step(int key){
	// check player movement
	//  set var to 1
	//  for each enemy
	//   if enemy in space we moving to
	//    set var to 0
	//    deal damage
	//  if var and space empty
	//   move
	// if player pressed E
	//  if tile is up stair
	//   ascend
	//   if last level
	//    gameend
	//  for all items
	//   if item under player
	//    remove item and place in inventory
	// foreach npc
	//  if abs(player_position-npc_position)<2
	//   deal damage to player
	//   check if hp zero
	//    if yes, do gameover
	//  else if can see player (XXX ???), move towards player
	//  we could also just have them move towards player when distance under some threshold? easier to implement . . .
	// if player pressed u
	//  XXX
	//    __16
	//  1/
	// 2@8
	//  4

	int nx=player.x,ny=player.y; // new player position
	switch(key){ // XXX this is gonna fuck us up later, but leave it for now
		case 1:nx--;break;
		case 2:ny--;break;
		case 3:nx++;break;
		case 4:ny++;break;
		default:break;
	}

	bool hit=true;
	for(int i=0;i<floors[current_floor].nc;i++){
		if(floors[current_floor].npcs[i].x==nx && floors[current_floor].npcs[i].y==ny && floors[current_floor].npcs[i].hp>0){
			hit=false;
			floors[current_floor].npcs[i].hp-=5; // XXX calc damage
			if(floors[current_floor].npcs[i].hp==0){
				player.lvl++;
				// motherfucker
				for(int j=i;j<floors[current_floor].nc;j++)
					floors[current_floor].npcs[j]=floors[current_floor].npcs[j+1];
				floors[current_floor].nc--;
				floors[current_floor].npcs = (Entity*)realloc(floors[current_floor].npcs,floors[current_floor].nc*sizeof(Entity));
			}
		}
	}
	if(hit && floors[current_floor].lvl[nx*mapsize+ny]!='#'){player.x=nx;player.y=ny;}

	if(key==5){ // XXX maybe use a switch statement instead? or we could go the hard way and use indices...
		// or first use indices for items, and then if not used se coords of stairs?
		if(floors[current_floor].lvl[player.x*mapsize+player.y]=='^'){
			// XXX do we want to compare against stored coords?
			// i mean, this is easier, but is it portable?
			current_floor++;
			player.x=floors[current_floor].in.x;
			player.y=floors[current_floor].in.y;
			if(current_floor==endor_floor){
				printf("Congratulations! You won the game!\n");
				exit(0);
			}
			//genlvl(current_floor);
		}
		else if(floors[current_floor].lvl[player.x*mapsize+player.y]=='v' && current_floor>0){
			current_floor--;
			player.x=floors[current_floor].out.x;
			player.y=floors[current_floor].out.y;
		}else // XXX pickup should go first!
		for(int i=0;i<floors[current_floor].ic;i++)
			if(floors[current_floor].items[i].x==player.x && floors[current_floor].items[i].y==player.y && player.items<max_inven && floors[current_floor].items[i].type!=Item::BLANK){
				player.items++;
				player.inven = (Item*)realloc(player.inven,player.items*sizeof(Item));
				player.inven[player.items-1]=floors[current_floor].items[i];
				for(int j=i;j<floors[current_floor].ic;j++)
					floors[current_floor].items[j]=floors[current_floor].items[j+1];
				floors[current_floor].ic--;
				floors[current_floor].items = (Item*)realloc(floors[current_floor].items,floors[current_floor].ic*sizeof(Item));

				player.hp+=5; // XXX remove when we introduce potions / healing
			}
	}

	if(key==6){ // XXX use item - do after overhaul
		printf("You look in your pockets, mumble mumble . . .\n");
	}

	for(int i=0;i<floors[current_floor].nc;i++)if(floors[current_floor].npcs[i].hp>0){
		int dx=floors[current_floor].npcs[i].x-player.x,dy=floors[current_floor].npcs[i].y-player.y;
		if(abs(dx)+abs(dy)<2){
			player.hp-=1; // XXX calculate damage dealt
			if(player.hp==0){
				// XXX plz do something more elegant . . .
				printf("You died.\n");
				exit(0);
			}
		}else{
			// XXX if LOS to player, move towards player
			// how will we do los?
			// we could just have them approach player...
			if(abs(dx)*abs(dx)+abs(dy)*abs(dy)<viewrad*viewrad&&abs(dx)>abs(dy)){ // XXX DO check collision, please?!?
				if(dx>0)floors[current_floor].npcs[i].x--;else floors[current_floor].npcs[i].x++;
			}else{
				if(dy>0)floors[current_floor].npcs[i].y--;else floors[current_floor].npcs[i].y++;
			}
		}
	}
}

void init(){
	// XXX cant we do the following in a constructor? do we want to?
	player.lvl=1;
	player.hp=200;
	player.items=0;
	player.inven=nullptr; // XXX that a thing?
	//for(int i=0;i<10;i++)player.inven[i]=nullptr; // we just dont go there

	Item*sword = new Item; // XXX must remain in memory!!!
	sword->x=sword->y=0;
	sword->type=Item::WEAPON;
	sword->durability=255;
	sword->stat=5;
	player.weapon=sword;

	Item*cloak = new Item;
	cloak->x=cloak->y=0;
	cloak->type=Item::ARMOR;
	cloak->durability=255;
	cloak->stat=10;
	player.armor=cloak;

	player.ring[0]=player.ring[1]=player.ring[2]=nullptr;

	for(int i=0;i<endor_floor;i++)genlvl(i);
	player.x=floors[0].in.x;
	player.y=floors[0].in.y;

}

void drop(Item*inven,int size,int n){
	if(n<size){ // i mean . . . kinda dumb, but better safe than sorry
		// XXX put back into world - cant do that w/ current codebase
		for(int i=n;i<size;i++){
			inven[i].type = inven[i+1].type;
			inven[i].durability = inven[i+1].durability;
			inven[i].stat = inven[i+1].stat;
		}
		// remove last - do we need to?
	}
	inven = (Item*)realloc(inven,size-1); // i hope i got this right

}

unsigned char disclaimer[]="\
                            License (OLC-3)\n\
                           ~~~~~~~~~~~~~~~\n\
              Copyright 2018 - 2019 OneLoneCoder.com\n\
Redistribution and use in source and binary forms, with or without modification\n\
, are permitted provided that the following conditions are met:\n\
\n\
  1. Redistributions or derivations of source code must retain the above\n\
copyright notice, this list of conditions and the following disclaimer.\n\
\n\
  2. Redistributions or derivative works in binary form must reproduce the above\n\
copyright notice. This list of conditions and the following disclaimer must be\n\
reproduced in the documentation and/or other materials provided with the\n\
distribution.\n\
\n\
  3. Neither the name of the copyright holder nor the names of its contributors\n\
may be used to endorse or promote products derived from this software without\n\
specific prior written permission.\n\
\n\
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n\
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n\
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n\
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE\n\
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n\
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR\n\
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER\n\
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n\
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n\
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";





//void inven(){}
	// im . . . not sure what to return?
	// we might also maybe wanna do it in main, so we can just render it on the go?
	// or we could do "void inven(void(draw)(char,int,int,int,int))" and call it w/ draw
	//  nvm i hate that idea
