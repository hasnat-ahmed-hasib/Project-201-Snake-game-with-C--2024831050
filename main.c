#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 500
#define HEIGHT 500
#define CELL_SIZE 20
#define ROWS (HEIGHT/CELL_SIZE)
#define COLS (WIDTH/CELL_SIZE)
#define LINE_WIDTH 2

// Converted hex colors to R, G, B, A format for the Renderer
#define COLOR_BLACK 0, 0, 0, 255
#define COLOR_WHITE 255, 255, 255, 255
#define GRID_COLOR 31, 31, 31, 255
#define COLOR_RED 255, 0, 0, 255
#define COLOR_GREEN 0, 255, 0, 255

// Macros updated to pass the renderer instead of psurface
#define SNAKE_HEAD(x,y) fill_cell(renderer, x, y, COLOR_GREEN);
#define SNAKE(x,y) fill_cell(renderer, x, y, COLOR_WHITE);
#define APPLE(x,y) fill_cell(renderer, x, y, COLOR_RED);
#define DRAW_GRID draw_grid(renderer)

/*Linked List*/
typedef struct SnakeElement {
    int x,y;
    struct SnakeElement *pnext;
} SnakeElement;

typedef struct Direction {
    int dx,dy;
} Direction;

typedef struct Apple {
    int x,y;
} Apple;

void insert_head(SnakeElement **psnake, Direction *dir)
{
    SnakeElement *newHead = malloc(sizeof(SnakeElement));

    if (newHead == NULL)
    {
        printf("Out of memory!\n");
        exit(1);
    }

    newHead->x = (*psnake)->x + dir->dx;
    newHead->y = (*psnake)->y + dir->dy;

    newHead->pnext = *psnake;

    *psnake = newHead;
}

void delete_tail(SnakeElement **psnake)
{
    if (*psnake == NULL)
        return;

    if ((*psnake)->pnext == NULL)
        return;

    SnakeElement *current = *psnake;

    while (current->pnext->pnext != NULL)
        current = current->pnext;

    free(current->pnext);
    current->pnext = NULL;
}

// Updated to use SDL_Renderer
void draw_grid(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, GRID_COLOR); // Set color once for the grid

    SDL_Rect row_line={0,0,WIDTH,LINE_WIDTH};
    for(row_line.y=0; row_line.y<HEIGHT; row_line.y+=CELL_SIZE){
        SDL_RenderFillRect(renderer, &row_line);
    }

    SDL_Rect col_line={0,0,LINE_WIDTH,HEIGHT};
    for(col_line.x=0; col_line.x<WIDTH; col_line.x+=CELL_SIZE){
        SDL_RenderFillRect(renderer, &col_line);
    }
}

// Updated to use SDL_Renderer and take individual R,G,B,A values
void fill_cell (SDL_Renderer *renderer, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a){
    SDL_Rect rect = {x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect (renderer, &rect);
}

// Updated to use SDL_Renderer
void draw_snake(SDL_Renderer *renderer, SnakeElement *psnake)
{
    if (psnake != NULL)
    {
        // Draw the head green
        SNAKE_HEAD(psnake->x, psnake->y);
        
        // Move to the next element to start drawing the body
        psnake = psnake->pnext;
    }
    while(psnake)
    {
        SNAKE(psnake->x,psnake->y);
        psnake = psnake->pnext;
    }
}

void move_snake(SnakeElement **psnake, Direction *dir)
{
    if ((*psnake)->pnext == NULL)
    {
        (*psnake)->x += dir->dx;
        (*psnake)->y += dir->dy;
        return;
    }

    insert_head(psnake, dir);
    delete_tail(psnake);
}

void reset_apple(SnakeElement* psnake, Apple* papple){
    papple->x = rand() % COLS;
    papple->y = rand() % ROWS;
    
    /*If apple coordinates collide with snake, try again*/
    SnakeElement* pcurrent = psnake;
    do {
        if(pcurrent->x == papple->x && pcurrent->y == papple->y){
            reset_apple(psnake, papple);
            break;
        }
        pcurrent = pcurrent->pnext;
    } while(pcurrent);
}

void grow_snake(SnakeElement **psnake, Direction *dir)
{
    if ((*psnake)->pnext == NULL)
    {
        insert_head(psnake, dir);
        return;
    }
    insert_head(psnake, dir);
}

int check_self_collision(SnakeElement *psnake, int targetX, int targetY) {
    SnakeElement *current = psnake;
    
    while (current != NULL) {
        if (current->x == targetX && current->y == targetY) {
            return 1; // Collision detected!
        }
        current = current->pnext;
    }
    return 0; // No collision
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color color = {255, 255, 255, 255}; // White color
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
void free_snake(SnakeElement *psnake) {
    while (psnake != NULL) {
        SnakeElement *temp = psnake;
        psnake = psnake->pnext;
        free(temp);
    }
}

// Call this to reset the game state
void reset_game(SnakeElement **psnake, int *score, Direction *dir, Apple *apple) {
    free_snake(*psnake);
    
    // Re-create starting snake (3 segments)
    *psnake = malloc(sizeof(SnakeElement));
    (*psnake)->x = 5; (*psnake)->y = 5;
    
    SnakeElement *body = malloc(sizeof(SnakeElement));
    body->x = 4; body->y = 5;
    
    SnakeElement *tail = malloc(sizeof(SnakeElement));
    tail->x = 3; tail->y = 5;
    
    (*psnake)->pnext = body;
    body->pnext = tail;
    tail->pnext = NULL;
    
    *score = 0;
    dir->dx = 1; dir->dy = 0;
    reset_apple(*psnake, apple);
}

int main(){
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    
    

    // Load font
    TTF_Font* font = TTF_OpenFont("ARIAL.TTF", 24); // 24 is the font size
   

    SDL_Window *window = SDL_CreateWindow(
        "Snake Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0
    );
    
   
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event event;
   
    Direction direction = {0,0};
    Direction* pdirection = &direction;
    Apple apple;
    Apple *papple = &apple;
    //Snake er First Coordinate
    // 1. Create the head
    SnakeElement *psnake = malloc(sizeof(SnakeElement));
    if (psnake == NULL) {
        printf("Out of memory!\n");
        exit(1);
    }
    psnake->x = 5;
    psnake->y = 5;

    // 2. Create the second segment (placed one cell to the left)
    SnakeElement *body = malloc(sizeof(SnakeElement));
    body->x = 4;
    body->y = 5;

    // 3. Create the third segment (placed two cells to the left)
    SnakeElement *tail = malloc(sizeof(SnakeElement));
    tail->x = 3;
    tail->y = 5;

    // 4. Link them all together: Head -> Body -> Tail -> NULL
    psnake->pnext = body;
    body->pnext = tail;
    tail->pnext = NULL;
    reset_apple(psnake, papple);

    int game = 1;
    int score = 0; 
    int is_game_over=0;           
    char title_buffer[64];    

   while(game) {
        // 1. EVENT HANDLING
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) game = 0;

            if(is_game_over) {
                // If game is over, only listen for 'R'
                if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                    reset_game(&psnake, &score, &direction, papple);
                    is_game_over = 0;
                }
            } else {
                // Listen for movement
                if(event.type == SDL_KEYDOWN) {
                    if(event.key.keysym.sym == SDLK_RIGHT && direction.dx != -1) { 
                        direction.dx = 1; 
                        direction.dy = 0; 
                    }
                    else if(event.key.keysym.sym == SDLK_LEFT && direction.dx != 1) { 
                        direction.dx = -1; 
                        direction.dy = 0; 
                    }
                    else if(event.key.keysym.sym == SDLK_UP && direction.dy != 1) { 
                        direction.dx = 0; 
                        direction.dy = -1; 
                    }
                    else if(event.key.keysym.sym == SDLK_DOWN && direction.dy != -1) { 
                        direction.dx = 0; 
                        direction.dy = 1; 
                    }
                }
            }
        }
        
        if(direction.dx == 0 && direction.dy == 0){
            direction.dx = 1;
        }
    
        // --- NEW: Clear the screen using the renderer
        SDL_SetRenderDrawColor(renderer, COLOR_BLACK);
        SDL_RenderClear(renderer); 

        int nextX = psnake->x + direction.dx;
        int nextY = psnake->y + direction.dy;

        // 2. GAME LOGIC
        if(!is_game_over) {
            int nextX = psnake->x + direction.dx;
            int nextY = psnake->y + direction.dy;

            if (nextX < 0 || nextX >= COLS || nextY < 0 || nextY >= ROWS || check_self_collision(psnake, nextX, nextY)) {
                is_game_over = 1;
            } else {
                if(nextX == papple->x && nextY == papple->y) {
                    grow_snake(&psnake, &direction);
                    reset_apple(psnake, papple);
                    score += 10;
                } else {
                    move_snake(&psnake, &direction);
                }
            }
        }
        
        if(is_game_over) {
            render_text(renderer, font, "GAME OVER - Press R to Restart", 80, 200);
        } else {
            APPLE(papple->x, papple->y);
            draw_snake(renderer, psnake);
            //DRAW_GRID;

            char score_text[32];
            snprintf(score_text, sizeof(score_text), "Score: %d", score);
            render_text(renderer, font, score_text, 10, 10);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(200);
    }
    
    // Good practice: Clean up the renderer before quitting
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}