/*

  nibbels: yet another snake rip
  Copyright (C) Chris Bush  2012

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
/*

  This program uses C++11, SDL2 and the outdated OpenGL. See http://libsdl.org.

*/
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>
#include <deque>

using std::deque;
using std::cout;

const int gamew = 160, gameh = 120, nodew = 16;
const int levelbreak = 640, maxlevel = 25;
const float node_rad = 4.0;
int level;

enum Direction {
  LEFT=1, RIGHT=2, UP=4, DOWN=8
};

void nextlevel();

struct node {
  float x, y;
  const float rad=node_rad;
  node(){}
  node(float _x, float _y):x(_x),y(_y){}
  void draw();
  
} orignode(gamew/2,gameh/2);

class Food : public node {
  public:
    char active;
    
  public:
    Food(){
      x = rand()%gamew*2-gamew;
      y = rand()%gameh*2-gameh;
      active = 1;
    }
    void update();
    
} *food;

class Snake {
  public:
    const int startlength = 10;
    float x, y;
    char xv, yv;
    char active;
    int length;
    deque<node*> tail;
    
  public:
    Snake():active(1),length(startlength),x(0.0),y(0.0),xv(0),yv(0){}
    ~Snake(){
      for(auto i : tail) delete i;
      tail.clear();
    }
    void update();
    void draw();
    void eat();
    void die();
    void go(Direction);
    int hits_self();
    void check_bounds();

} *snake;

int main(int argc, char* argv[]){

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window* window = SDL_CreateWindow(
    "nibbels",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    640,
    480,
    SDL_WINDOW_OPENGL
  );
  
  SDL_GLContext glcontext = SDL_GL_CreateContext(window);

  glFrustum(-gamew,gamew,gameh,-gameh,1,5);
  
  snake = new Snake();
  food = new Food();
  level = 1;
  
  for(SDL_Event e; e.type!=SDL_QUIT; SDL_PollEvent(&e)){
    
    if(!snake->active){  
      delete snake;
      snake = new Snake();
    
    }
    if(!food->active){
      delete food;
      food = new Food();
    
    }
  
    if(e.type==SDL_KEYDOWN){
      switch(e.key.keysym.sym){
        case SDLK_UP:    snake->go(UP);    break;
        case SDLK_DOWN:  snake->go(DOWN);  break;
        case SDLK_LEFT:  snake->go(LEFT);  break;
        case SDLK_RIGHT: snake->go(RIGHT); break;
        case SDLK_ESCAPE: e.type=SDL_QUIT;  break;
      }
    }
    
    glClearColor(
      sin((float)level+1)/2+0.5,
      sin((float)level+2)/2+0.5,
      sin((float)level+3)/2+0.5,
      1
    );
      
    glClear(GL_COLOR_BUFFER_BIT);

    food->update();
    snake->update();
    
    SDL_GL_SwapWindow(window);
    SDL_Delay(5+maxlevel-level);
    
  }
  
  SDL_GL_DeleteContext(glcontext);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
  
}

void Snake::eat(){
  const static int rad2 = node_rad*node_rad;
  if((x-food->x)*(x-food->x)+(y-food->y)*(y-food->y) < rad2){
    food->active = 0;
    if((length *= 2) > levelbreak){
      nextlevel();
      length = startlength;
    }
  
  }
}

void Snake::draw(){
  
  glColor3f(
    cos((float)level+1)/2+0.5,
    cos((float)level+2)/2+0.5,
    cos((float)level+3)/2+0.5
  );
  
  glBegin(GL_QUADS);
  
  for(node* i : tail)
    i->draw();
 
  glEnd();

}

void Snake::check_bounds(){
  if(x < -gamew) x = gamew;   else
  if(x > gamew) x = -gamew;   else
  if(y < -gameh) y = gameh;   else
  if(y > gameh) y = -gameh;
}

void Snake::go(Direction d){
  switch(d){
    case UP:    if(yv > 0) return;  xv = 0; yv = -1;  break;
    case DOWN:  if(yv < 0) return;  xv = 0; yv =  1;  break;
    case LEFT:  if(xv > 0) return;  yv = 0; xv = -1;  break;
    case RIGHT: if(xv < 0) return;  yv = 0; xv =  1;  break;
  }
}

void Snake::die(){  
  active = 0;
}

int Snake::hits_self(){
  
  if(!xv&&!yv) return 0;
  for(int i=0; i < (int)tail.size()-6; ++i){
    
    node* n = tail[i];
    float rad2 = n->rad*n->rad;
    if((x-n->x)*(x-n->x)+(y-n->y)*(y-n->y) < rad2) return 1;
    
  }
  return 0;
  
}

void Snake::update(){

  static int i=0;
  
  if(!active) return;

  if(xv||yv){
    x += xv*node_rad;
    y += yv*node_rad;
    check_bounds();

    if(hits_self()){
      die();
      return;
    }
    
    eat();
    
    tail.push_back(new node(x,y)); 
    while(tail.size() > length){ 
      delete tail.front();
      tail.pop_front();
    }
  }
  
  draw();
  
}

void Food::update(){
  
  if(!active) return;
  
  glColor3f(
    sin((float)level+3)/2+0.5,
    sin((float)level+1)/2+0.5,
    sin((float)level+2)/2+0.5
  );
  glBegin(GL_QUADS);
  draw();
  glEnd();
  
}

void node::draw(){
  glVertex3f(x-rad/2,y-rad/2,-1);
  glVertex3f(x-rad/2,y+rad/2,-1);
  glVertex3f(x+rad/2,y+rad/2,-1);
  glVertex3f(x+rad/2,y-rad/2,-1);
}

void nextlevel(){

  if(++level > maxlevel) level = 1; 

}
