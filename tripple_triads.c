#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define maxNameLength 20
//global variables
static int cardcount = 501;
static int actualCardCount = 0;
static int enemyCardCount = 5;
static int maxDepth = 8;
static int expectedValue = 0;

//structs
typedef struct board {
    int cards[9];   //initialize all to -1
    int ownedByPlayer[9]; //initialize to -1
} Board;

typedef struct move {
    float value;
    int card;
    int field;
} Move;

typedef struct card {
    int up;
    int down;
    int right;
    int left;
    char name[maxNameLength];
} Card;
static Card* noCard;
//functions
static Card* loadcards(void) {
    fprintf(stdout, "impotring cards...\n");
    struct card* cards = malloc(sizeof(struct card) * cardcount);
    FILE* cardfile;
    char line[500];
    if (!cards) {
        fprintf(stderr, "Failed to allocate space for card array; aborting program");
        exit(EXIT_FAILURE);
    }

    noCard = cards;
    cards = &cards[1];
    {
        noCard->up = 0;
        noCard->down = 0;
        noCard->right = 0;
        noCard->left = 0;
    }

    //load cards from file
    cardfile = fopen("cards.data", "r");
    if (!cardfile) {
        fprintf(stderr, "Failed to open card_file; aborting");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (fgets(line, 500, cardfile)){
        Card newCard;
        char *name = strtok(line, "\"");
        if (strlen(name) > maxNameLength){
            fprintf(stderr, "Name of card to long, cardname: %s", name);
            fprintf(stderr, "using first %i chars as name", maxNameLength);
        }
        memcpy(newCard.name, name, maxNameLength);
        newCard.name[maxNameLength - 1] = '\0';

        newCard.up = atoi(strtok(NULL, " "));
        newCard.down = atoi(strtok(NULL, " "));
        newCard.right = atoi(strtok(NULL, " "));
        newCard.left = atoi(strtok(NULL, " "));
        cards[i++] = newCard;
    }
    actualCardCount = i;
    fprintf(stdout, "%i cards loaded successfully\n", actualCardCount);
    fclose(cardfile);
    return cards;
}

static int* loadPlayerCards( Card *cardlist){
    //cardlist isn't changed in this function, but can't be const since pointers are saved to output
    fprintf(stdout, "loading player cards...\n");
    int *playerCards = malloc(sizeof(int)*5);
    
    if (!playerCards) {
        fprintf(stderr, "Failed to allocate space for playerCard array; aborting program");
        exit(EXIT_FAILURE);
    }

    FILE* playerFile;
    char line[500];

    playerFile = fopen("playerCards.data", "r");
    if (!playerFile) {
        fprintf(stderr, "Failed to open player_file; aborting");
        exit(EXIT_FAILURE);
    }

    int cardCounter = 0;
    while (fgets(line, 500, playerFile)){
        if (cardCounter > 5){
            fprintf(stderr, "Player is expected to have 5 cards; found more in list; exiting");
            exit(EXIT_FAILURE);
        }
        playerCards[cardCounter] = -1; //initializes index
        line[strcspn(line, "\n")] = '\0'; //removes \n since names are saved without
        for (int i = 0; i<actualCardCount; i++){
            if (strcmp(line, cardlist[i].name) == 0) {
                playerCards[cardCounter] = i;
                break; //break since card is allready found; no need to look through rest
            }
        }
        if (playerCards[cardCounter] == -1) {
            fprintf(stderr, "Couldn't find card \"%s\" in cardlist; exiting", line);
            exit(EXIT_FAILURE);
        }
        cardCounter++;
    }

    if (cardCounter < 5) {
        fprintf(stderr, "Player is expected to have 5 cards; not enough cards; exiting");
        exit(EXIT_FAILURE);
    }

    //test for duplicates (they are actually legal)
    /*
    for (int i = 0; i < 4; i++){
        for (int j = i + 1; j < 5; j++){
            if (playerCards[i] == playerCards[j]) {
                fprintf(stderr, "\"%s\" duplicate; invalid; exiting", cardlist[playerCards[i]].name);
                exit(EXIT_FAILURE);
            }
        }
    }
    */
    fprintf(stdout, "player cards loaded successfully\n");
    fclose(playerFile);
    return playerCards;
}

static int* loadEnemyCards( Card *cardlist){
    //cardlist isn't changed in this function, but can't be const since pointers are saved to output
    fprintf(stdout, "loading enemy cards...\n");
    int*enemyCards = malloc(sizeof(int)*5);
    
    if (!enemyCards) {
        fprintf(stderr, "Failed to allocate space for enemyCard array; aborting program");
        exit(EXIT_FAILURE);
    }

    FILE* enemyFile;
    char line[500];

    enemyFile = fopen("enemyCards.data", "r");
    if (!enemyFile) {
        fprintf(stderr, "Failed to open enemy_file; aborting");
        exit(EXIT_FAILURE);
    }

    int cardCounter = 0;
    while (fgets(line, 500, enemyFile)){
        if (cardCounter == 0) {
            line[strcspn(line, "\n")] = '\0';
            if (strcmp(line, "all") == 0){
                fprintf(stdout, "enemy cards set to all\n");
                maxDepth = 8 - actualCardCount/10;
                if (maxDepth > 8) maxDepth = 8;
                if (maxDepth < 1) maxDepth = 1;
                enemyCardCount = actualCardCount;

                enemyCards = realloc(enemyCards, sizeof(int)*actualCardCount);
                if (!enemyCards) {
                fprintf(stderr, "Failed to reallocate space for enemyCard array; aborting program");
                exit(EXIT_FAILURE);
                }

                for (int i = 0; i<actualCardCount; i++){
                    enemyCards[i] = i;
                }
                fprintf(stdout, "enemy cards loaded successfully\n");
                fclose(enemyFile);
                return enemyCards;
            }
        }

        if (cardCounter > 5){
            fprintf(stderr, "Player is expected to have 5 cards; found more in list; exiting");
            exit(EXIT_FAILURE);
        }
        enemyCards[cardCounter] = -1; //initializes index
        line[strcspn(line, "\n")] = '\0'; //removes \n since names are saved without
        for (int i = 0; i<actualCardCount; i++){
            if (strcmp(line, cardlist[i].name) == 0) {
                enemyCards[cardCounter] = i;
                break; //break since card is allready found; no need to look through rest
            }
        }
        if (enemyCards[cardCounter] == -1) {
            fprintf(stderr, "Couldn't find card \"%s\" in cardlist; exiting", line);
            exit(EXIT_FAILURE);
        }
        cardCounter++;
    }

    if (cardCounter < 5) {
        fprintf(stderr, "Player is expected to have 5 cards; not enough cards; exiting");
        exit(EXIT_FAILURE);
    }

    //test for duplicates (actually legal)
    /*
    for (int i = 0; i < 4; i++){
        for (int j = i + 1; j < 5; j++){
            if (enemyCards[i] == enemyCards[j]) {
                fprintf(stderr, "\"%s\" duplicate; invalid; exiting", cardlist[enemyCards[i]].name);
                exit(EXIT_FAILURE);
            }
        }
    }
*/
    fprintf(stdout, "enemy cards loaded successfully\n");
    fclose(enemyFile);
    return enemyCards;
}

static Board updateBoard(Board board, int fieldToPlace, int cardID, int playerTurn, Card cards[]){
    /*  Tested before function call
    if (board.cards[fieldToPlace] != -1) {

        fprintf(stderr, "field allready occupied");
        exit(EXIT_FAILURE);
    }
    */
    board.cards[fieldToPlace] = cardID;
    board.ownedByPlayer[fieldToPlace] = playerTurn;

    if (fieldToPlace - 1 >= 0 && (fieldToPlace - 1)%3 != 2 && board.cards[fieldToPlace - 1] != -1){ //card to the left
        if (cards[board.cards[fieldToPlace - 1]].right < cards[cardID].left) board.ownedByPlayer[fieldToPlace - 1] = playerTurn;
    }
    if (fieldToPlace - 3 >= 0 && board.cards[fieldToPlace - 3] != -1){ //field above
        if (cards[board.cards[fieldToPlace - 3]].down < cards[cardID].up) board.ownedByPlayer[fieldToPlace - 3] = playerTurn;
    }
    if (fieldToPlace + 1 < 9 && (fieldToPlace + 1)%3 != 0 &&  board.cards[fieldToPlace + 1] != -1) { //field to the right
        if (cards[board.cards[fieldToPlace + 1]].left < cards[cardID].right) board.ownedByPlayer[fieldToPlace + 1] = playerTurn;
    }
    if (fieldToPlace + 3 < 9 && board.cards[fieldToPlace + 3] != -1){ //field below
        if (cards[board.cards[fieldToPlace + 3]].up < cards[cardID].down) board.ownedByPlayer[fieldToPlace + 3] = playerTurn;
    }

    return board;
}

static float evalBoard(Board board, int enemyCards[],int eCardCount,int playerCards[], int pCardCount, Card cards[]){
    float value = -1;
    for (int field = 0; field < 9; field++){
        if (board.cards[field] == -1) continue; //skip if field is unoccupied
        if (board.ownedByPlayer[field]){
            //add player value
            value += 1;
            if (field + 1 < 9 && (field + 1)%3 != 0 && board.cards[field + 1] == -1){
                // field to the right
                float losses = 0;
                for (int i = 0; i < eCardCount; i++){
                    if (cards[enemyCards[i]].left > cards[board.cards[field]].right) losses++;
                }
                value -= losses/eCardCount;
            }
            if (field - 1 >= 0 && (field - 1)%3 != 2 && board.cards[field - 1] == -1){
                // field to the left
                float losses = 0;
                for (int i = 0; i < eCardCount; i++){
                    if (cards[enemyCards[i]].right > cards[board.cards[field]].left) losses++;
                }
                value -= losses/eCardCount;
            }
            if (field - 3 >= 0 && board.cards[field - 3] == -1){
                // field above
                float losses = 0;
                for (int i = 0; i < eCardCount; i++){
                    if (cards[enemyCards[i]].down > cards[board.cards[field]].up) losses++;
                }
                value -= losses/eCardCount;
            }
            if (field + 3 < 9 && board.cards[field + 3] == -1){
                // field below
                float losses = 0;
                for (int i = 0; i < eCardCount; i++){
                    if (cards[enemyCards[i]].up > cards[board.cards[field]].down) losses++;
                }
                value -= losses/eCardCount;
            }
        }
        else {
            //subtract enemy value
            value -= 1;
            if (field + 1 < 9 && (field + 1)%3 != 0 && board.cards[field + 1] == -1){
                // field to the right
                float losses = 0;
                for (int i = 0; i < pCardCount; i++){
                    if (cards[playerCards[i]].left > cards[board.cards[field]].right) losses++;
                }
                value += losses/pCardCount;
            }
            if (field - 1 >= 0 && (field - 1)%3 != 2 && board.cards[field - 1] == -1){
                // field to the left
                float losses = 0;
                for (int i = 0; i < pCardCount; i++){
                    if (cards[playerCards[i]].right > cards[board.cards[field]].left) losses++;
                }
                value += losses/pCardCount;
            }
            if (field - 3 >= 0 && board.cards[field - 3] == -1){
                // field above
                float losses = 0;
                for (int i = 0; i < pCardCount; i++){
                    if (cards[playerCards[i]].down > cards[board.cards[field]].up) losses++;
                }
                value += losses/pCardCount;
            }
            if (field + 3 < 9 && board.cards[field + 3] == -1){
                // field below
                float losses = 0;
                for (int i = 0; i < pCardCount; i++){
                    if (cards[playerCards[i]].up > cards[board.cards[field]].down) losses++;
                }
                value += losses/pCardCount;
            }
        }
    }
    return value;
}

void printBoard(Board board, Card* cards){  //not critical; no need to optimize
    printf(" \n\n");
    for (int i = 0; i < 9; i += 3){
        printf("   %*i         %*i         %*i\n", 2, cards[board.cards[i]].up, 2, cards[board.cards[i+ 1]].up, 2, cards[board.cards[i+ 2]].up);
        printf("%*i %*i %*i   %*i %*i %*i   %*i %*i %*i\n", 2, cards[board.cards[i]].left, 2, board.ownedByPlayer[i], 2, cards[board.cards[i]].right,2, cards[board.cards[i +1]].left, 2, board.ownedByPlayer[i +1], 2, cards[board.cards[i +1]].right,2, cards[board.cards[i +2]].left, 2, board.ownedByPlayer[i +2], 2, cards[board.cards[i +2]].right);
        printf("   %*i         %*i         %*i\n", 2, cards[board.cards[i]].down, 2, cards[board.cards[i+ 1]].down, 2, cards[board.cards[i+ 2]].down);
        printf("\n");
    }
    // printf(" \n\n");
    // for (int i = 0; i < 9; i++){
    //     if (i == 0 || i == 3 || i == 6) printf(" \n");
    //     printf(" %*i  ",2, board.ownedByPlayer[i]);
    // }
    printf("\n\n");
}

void printCard(int card, int num, Card* cards){
    printf("                            %*i\n", 2, cards[card].up);
    printf("%*i: %-*s %*i %*i %*i\n",2, num, 20, cards[card].name, 2, cards[card].left, 2, card, 2, cards[card].right);
    printf("                            %*i\n\n", 2, cards[card].down);
}

static Move minimax(Board board, int playerCards[], int enemyCards[], int pCardCount, int eCardCount, Card cards[], int turn, int maxDepth,float alpha , float beta, bool playerTurn){
    Move move;

    float bestVal;
    if (turn == maxDepth){  //return if maxDepth
        move.value = evalBoard(board, enemyCards, eCardCount, playerCards, pCardCount, cards);
        //printBoard(board, cards);
        //printf("%f\n\n", move.value);
        //while (!getchar());
        return move;
    }
    if (playerTurn){
        bestVal = -1000;  //initialize to something lower than expected values for maxsearch
        for (int i = 0; i<pCardCount; i++){ //for all player Cards
            int newPlayerCards[5];
            memcpy(newPlayerCards, playerCards, (pCardCount) << 2);   //duplicate array
            // sizeof(int) == 4; only need to copy first <pCardCount> - 1 Elements
            int cardToPlay = newPlayerCards[i];     //pull choosen card from deck
            newPlayerCards[i] = playerCards[pCardCount-1];
            //DEBUG
                //if (turn == 0) printf("TESTING CARD %s\n", cards[cardToPlay].name);
            for (int j = 0; j<9; j++){
                if (board.cards[j] != -1) continue; //skip if field is allready taken
                Board newBoard = updateBoard(board, j, cardToPlay, 1, cards);
                float eval = minimax(newBoard, newPlayerCards, enemyCards, pCardCount-1, eCardCount, cards, turn + 1, maxDepth, alpha, beta, 0).value;
                
                if (eval > bestVal) {
                    bestVal = eval;
                    move.value = eval;
                    move.card = i;
                    move.field = j; 
                }

                // alpah pruning
                if (alpha < eval) alpha = eval;
                if (beta <= alpha) goto RET;
            }
        }
    }
    else{
       bestVal = 1000;
        for (int i = 0; i<eCardCount; i++){
            int newEnemyCards[cardcount];
            memcpy(newEnemyCards, enemyCards, (eCardCount) << 2);
            // sizeof(int) == 4; only need to copy first <eCardCount> - 1 Elements
            int cardToPlay = newEnemyCards[i];
            newEnemyCards[i] = enemyCards[eCardCount-1];
            for (int j = 0; j<9; j++){
                if (board.cards[j] != -1) continue; //skip if field is allready taken
                Board newBoard = updateBoard(board, j, cardToPlay, 0, cards);
                float eval = minimax(newBoard, playerCards, newEnemyCards, pCardCount, eCardCount - 1, cards, turn + 1, maxDepth, alpha, beta, 1).value;

                if (eval < bestVal) {
                    bestVal = eval;
                    move.value = eval;
                    //don't need to set rest since I don't need to know best move for enemy
                }

                // beta pruning
                if (beta > eval) beta = eval;
                if (beta <= alpha) goto RET;

                
            }
        }
    }
    RET:;
    return move;
}


static void playGame(int pCards[], int eCards[], int eCardCount, int pCardCount, Card cards[]){
    int turnNumber = 0;
    int playerturn = -1;

    //init board
    Board board;
    for (int i = 0; i<9; i++){
        board.cards[i] = -1;
        board.ownedByPlayer[i] = -1;
    }

    printf("Enter 1 if it is player's turn, else it's enemy turn\n");
    char input[10];
    playerturn = strtol(fgets(input, 10, stdin), NULL, 10);
    printf("\nSettup Complete\nPress key to continue...\n");
    while (!getchar());
    system("cls");
    
    while (turnNumber < 9) {
        if( maxDepth > 8 - turnNumber){ // cap Maxdepth at remaining possible turns
            maxDepth = 8 - turnNumber;  	//turns capped to 8 since the ninth is "forced"
        }

        if (playerturn) {
            printf("Player Turn\n");
            printf("Calculating moves (this might take a while)\n");
            Move move = minimax(board, pCards, eCards, pCardCount, eCardCount, cards, 0, maxDepth, -10000000000, 10000000000, 1);
            expectedValue = move.value;
            printf("Reccomended Move: \"%s\" on field %i; value: %f\n", cards[pCards[move.card]].name, move.field, move.value);
            maxDepth += turnNumber; //Able to increase since a lot of possibillities get lost each round
        }
        else {
            printf("\n\nEnemy Turn\n");
        }
        
        printf("Card index:\n");
        if (playerturn) {
            for (int i = 0; i<pCardCount; i++) {
                //printf("%*i: %s\n", 2, i, cards[pCards[i]].name);
                printCard(pCards[i], i, cards);
            }
        }
        else {
            for (int i = 0; i<eCardCount; i++) {
                //printf("%*i: %s\n", 2, i, cards[eCards[i]].name);
                printCard(eCards[i], i, cards);
            }
        }
        
        cardInput:
        //get input
        printf("\nEneter Card to play\n");
        char input[10];
        fgets(input, 10, stdin);
        int ctp = strtol(input, NULL, 10);
        printf("Eneter Field to place\n");
        fgets(input, 10, stdin);
        int ftp = strtol(input, NULL, 10);

        //input handeling
        if (playerturn){
            if (ctp >= pCardCount){
                printf("Card Index out of bounds; exiting\n");
                goto cardInput;
            }
        }
        else {
            if (ctp >= eCardCount){
                printf("Card Index out of bounds; Try again\n");
                goto cardInput;
            }
        }

        if (ftp < 0 || ftp > 8) {
            printf("Field not on board; Try again\n");
            goto cardInput;
        }
        else {
            if (board.cards[ftp] != -1) {
                printf("Field occupied; Try again\n");
                goto cardInput;
            }
        }
        
        //doTurn
        //remove card from deck
        if (playerturn){
            int card = pCards[ctp];
            pCards[ctp] = pCards[pCardCount - 1];
            pCardCount--;
            ctp = card;
        }
        else{
            int card = eCards[ctp];
            eCards[ctp] = eCards[eCardCount - 1];
            eCardCount--;
            ctp = card;
        }

        //clear screen for next round
        system("cls");
        board = updateBoard(board, ftp, ctp, playerturn, cards);
        printBoard(board, cards);   //print board for "user interface"
        printf("Expected End Value: %i\n", expectedValue);
        turnNumber++;
        if (playerturn) playerturn = 0;
        else playerturn = 1;
    }
    printf("\nGame over...\n");
    printf("press enter to quit\n");
    while (!getchar());
    
}
//main
int main(int argc, char const *argv[])
{
    Card *cards = loadcards();
    int *playerCards = loadPlayerCards(cards);
    int *enemyCards = loadEnemyCards(cards);
    //player-/enemy-Cards use index in cards array as ID (for storage and quick lookup)

    //args
    if (argc > 1){
        int turnCount = strtol(argv[1],NULL, 10);
        if (0 < turnCount || turnCount <= 9){
            maxDepth = turnCount;
            printf("MaxDepth set to %i\n", maxDepth);
        }
        else {
            printf("MaxDepth needs to be between 0 and 9; keeping standard (5)\n");
        }
    }
    printf("MaxDepth is set to %i\n", maxDepth);

    printf("\nPress key to continue...\n");
    while (!getchar());
    system("cls");
    playGame(playerCards, enemyCards, enemyCardCount, 5, cards);

    exit(EXIT_SUCCESS);
}