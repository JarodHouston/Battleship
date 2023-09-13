//
//  main.cpp
//  Battleship
//
//  Created by Jarod Houston on 12/16/22.
//

#include <iostream>
#include <string>
#include <random>
#include <utility>
#include <cctype>
using namespace std;


///////////////////////////////////////////////////////////////////////////
// Manifest constants
///////////////////////////////////////////////////////////////////////////

const int MAXROWS = 10;
const int MAXCOLS = 10;
const int MAXSHIPS = 5;
const int MAXSHIPLENGTH = 5;

const int DESTROYER = 0;
const int SUBMARINE = 1;
const int CRUISER = 2;
const int BATTLESHIP = 3;
const int CARRIER = 4;

const int EMPTY = 0;
const int OCCUPIED = 1;
const int DAMAGED = 2;
const int HIT = 3;
const int NONE = 4;

const int HORIZONTAL = 0;
const int VERTICAL = 1;

const int SPACE = 20;

const int NORTH = 0;
const int EAST = 1;
const int SOUTH = 2;
const int WEST = 3;

///////////////////////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////////////////////


class Grid;

class Ship {
public:
      // Constructor
    Ship(Grid* grid, string name, string coord, int orientation, int length);
    
      // Accessors
    int length() const;
    string name() const;
    string coordinate() const;
    int orientation() const;
    bool hasSunk() const;
    
      // Mutators
    void setStatus(int row, int col, int status);
    
private:
    Grid* m_grid;
    string m_name;
    string m_coord;
    int m_orientation;
    int m_length;
    int m_status[MAXSHIPLENGTH];
    
      // Helper functions
};

class Grid {
public:
      // Constructor/destructor
    Grid(bool isOpponent);
    ~Grid();
    
      // Accessors
    int rows() const;
    int cols() const;
    int numShips() const;
    bool isOpponent() const;
    int getCellStatus(int row, int col) const;
    
      // Mutators
    void addShip(string name, string coord, int orientation, int length);
    Ship* hitShip(int row, int col);
    void sinkShip(Ship* ship);
    void setCellStatus(int row, int col, int status);
    void displayGrid();
    
private:
    int m_grid[MAXROWS][MAXCOLS];
    int m_rows;
    int m_cols;
    int m_numShips;
    bool m_isOpponent;
    Ship* m_ships[MAXSHIPS];
    
      // Helper functions
    
};


///////////////////////////////////////////////////////////////////////////
//  Auxiliary function declarations
///////////////////////////////////////////////////////////////////////////


int randInt(int lowest, int highest);
bool isValidCoord(string coord);
bool isValidShipPos(Grid* g, string coord, int length, int orientation);
int decodeOrientation(string orientation);
int rowOfCoord(string coord);
int colOfCoord(string coord);
char getLetter(int n);
void printNums();
void display(Grid* player, Grid* opponent);
void addOppShips(Grid* g);
void attack(Grid* g, int row, int col);


///////////////////////////////////////////////////////////////////////////
//  Grid implementation
///////////////////////////////////////////////////////////////////////////

Grid::Grid(bool isOpponent) {
    m_rows = MAXROWS;
    m_cols = MAXCOLS;
    m_numShips = 0;
    m_isOpponent = isOpponent;
    
    for (int i = 0; i < MAXSHIPS; i++) {
        m_ships[i] = nullptr;
    }
    
    for (int i = 0; i < m_rows; i++) {
        for (int j = 0; j < m_cols; j++) {
            m_grid[i][j] = EMPTY;
        }
    }
}

Grid::~Grid() {
    for (int i = 0; i < MAXSHIPS; i++) {
        delete m_ships[i];
    }
}

int Grid::rows() const {
    return m_rows;
}

int Grid::cols() const {
    return m_cols;
}

int Grid::numShips() const {
    return m_numShips;
}

bool Grid::isOpponent() const {
    return m_isOpponent;
}

int Grid::getCellStatus(int row, int col) const{
    return m_grid[row - 1][col - 1];
}


void Grid::addShip(string name, string coord, int orientation, int length) {
    m_ships[m_numShips] = new Ship(this, name, coord, orientation, length);
    m_numShips++;
    
    if (orientation == HORIZONTAL) {
        for (int i = 0; i < length; i++) {
            setCellStatus(rowOfCoord(coord), colOfCoord(coord) + i, OCCUPIED);
        }
    }
    if (orientation == VERTICAL) {
        for (int i = 0; i < length; i++) {
            setCellStatus(rowOfCoord(coord) + i, colOfCoord(coord), OCCUPIED);
        }
    }
}

Ship* Grid::hitShip(int row, int col) {
    for (int i = 0; i < m_numShips; i++) {
        for (int j = 0; j < m_ships[i]->length(); j++) {
            int r = 0;
            int c = 0;
            if (m_ships[i]->orientation() == HORIZONTAL) {
                c = j;
            }
            if (m_ships[i]->orientation() == VERTICAL) {
                r = j;
            }
            if (row == rowOfCoord(m_ships[i]->coordinate()) + r && col == colOfCoord(m_ships[i]->coordinate()) + c) {
                m_ships[i]->setStatus(row, col, DAMAGED);
                //cout << "...hitting the " << m_ships[i]->name() << "..." << endl;
                return m_ships[i];
            }
        }
    }
    return nullptr;
}

void Grid::sinkShip(Ship* ship) {
    for (int i = 0; i < m_numShips; i++) {
        if (m_ships[i] == ship) {
            delete m_ships[i];
            m_ships[i] = m_ships[m_numShips - 1];
            m_numShips--;
        }
    }
}

void Grid::setCellStatus(int row, int col, int status) {
    if (row == -1 || col == -1) {
        cout << "***** Attempted to set cell status on an invalid coordinate!" << endl;
        return;
    }
    
    m_grid[row - 1][col - 1] = status;
}

void Grid::displayGrid() {
    cout << "  ";
    printNums();
    cout << endl;
    
    for (int i = 0; i < MAXROWS; i++) {
        cout << getLetter(i + 1) << " ";
        for (int j = 0; j < MAXCOLS; j++) {
            if (m_grid[i][j] == EMPTY || (m_isOpponent && m_grid[i][j] == OCCUPIED)) {
                cout << ". ";
            }
            else if (m_grid[i][j] == OCCUPIED) {
                cout << "O ";
            }
            else if (m_grid[i][j] == HIT) {
                cout << "  ";
            }
            else {
                cout << "X ";
            }
        }
        
        cout << endl;
    }
}


///////////////////////////////////////////////////////////////////////////
//  Ship implementation
///////////////////////////////////////////////////////////////////////////


Ship::Ship(Grid* grid, string name, string coord, int orientation, int length) {
    if (grid == nullptr) {
        cout << "***** The ship must be created on some Grid!" << endl;
        exit(1);
    }
    if (!isValidCoord(coord)) {
        cout << "***** Ship was created with invalid coordinates!" << endl;
        exit(1);
    }
    if (orientation > 1 || orientation < 0) {
        cout << "***** Ship must be either horizontal or vertical!" << endl;
        exit(1);
    }
    if (length < 0 || length > MAXSHIPLENGTH) {
        cout << "***** Invalid length of ship!" << endl;
        exit(1);
    }
    
    m_grid = grid;
    m_name = name;
    m_coord = coord;
    m_orientation = orientation;
    m_length = length;
    
    for (int i = 0; i < m_length; i++) {
        m_status[i] = OCCUPIED;
    }
}

int Ship::length() const {
    return m_length;
}
string Ship::name() const  {
    return m_name;
}
string Ship::coordinate() const {
    return m_coord;
}
int Ship::orientation() const {
    return m_orientation;
}
bool Ship::hasSunk() const {
    for (int i = 0; i < m_length; i++) {
        if (m_status[i] != DAMAGED) {
            return false;
        }
    }
    return true;
}

void Ship::setStatus(int row, int col, int status) {
    int i = 0;
    if (m_orientation == HORIZONTAL) {
        i = col - colOfCoord(m_coord);
    }
    else {
        i = row - rowOfCoord(m_coord);
    }
    //cout << m_coord << " " << rowOfCoord(m_coord) << ", " << colOfCoord(m_coord) << endl;
    //cout << row << col << " " << i << endl;
    m_status[i] = status;
}


///////////////////////////////////////////////////////////////////////////
//  Auxiliary functions
///////////////////////////////////////////////////////////////////////////


int randInt(int lowest, int highest)
{
    if (highest < lowest)
        swap(highest, lowest);
    static random_device rd;
    static default_random_engine generator(rd());
    uniform_int_distribution<> distro(lowest, highest);
    return distro(generator);
}


bool isValidCoord(string coord) {
    if (coord.length() < 2 || coord.length() > 3 || !isalpha(coord.at(0)) || !isdigit(coord.at(1))) {
        //cout << "Invalid coordinates" << endl;
        return false;
    }
    if (coord.at(0) == '#') {
        return false;
    }
    
    char c = tolower(coord.at(0));
    int num;
    if (coord.length() == 2) {
        num = coord.at(1) - 48;
    }
    else {
        num = (coord.at(1) - 48)*10 + (coord.at(2) - 48);
    }
    
    if (c > 96 + MAXROWS || num <= 0 || num > MAXCOLS) {
        //cout << "Invalid coordinates" << endl;
        return false;
    }
    
    return true;
}

bool isValidShipPos(Grid* g, string coord, int length, int orientation) {
    
    if (orientation == HORIZONTAL) {
        if (colOfCoord(coord) + length > MAXCOLS + 1) {
            return false;
        }
        for (int i = 0; i < length; i++) {
            if (g->getCellStatus(rowOfCoord(coord), colOfCoord(coord) + i) != EMPTY) {
                return false;
            }
        }
        int num = -1;
        while (num < 2) {
            for (int j = colOfCoord(coord) - 1; j < colOfCoord(coord) + length + 1; j++) {
                string newCoord = getLetter(rowOfCoord(coord) + num) + to_string(j);
                if (!isValidCoord(newCoord)) {
                    continue;
                }
                if (num == 0 && j >= colOfCoord(coord) && j < colOfCoord(coord) + length) {
                    continue;
                }
                if (g->getCellStatus(rowOfCoord(coord) + num, j) != EMPTY) {
                    return false;
                }
            }
            num++;
        }
    }
    if (orientation == VERTICAL) {
        if (rowOfCoord(coord) + length > MAXROWS + 1) {
            return false;
        }
        for (int i = 0; i < length; i++) {
            if (g->getCellStatus(rowOfCoord(coord) + i, colOfCoord(coord)) != EMPTY) {
                return false;
            }
        }
        int num = -1;
        while (num < 2) {
            for (int j = rowOfCoord(coord) - 1; j < rowOfCoord(coord) + length + 1; j++) {
                string newCoord = getLetter(j) + to_string(colOfCoord(coord) + num);
                if (!isValidCoord(newCoord)) {
                    continue;
                }
                if (num == 0 && j >= rowOfCoord(coord) && j < rowOfCoord(coord) + length) {
                    continue;
                }
                if (g->getCellStatus(j, colOfCoord(coord) + num) != EMPTY) {
                    return false;
                }
            }
            num++;
        }
    }
    
    return true;
}

int decodeOrientation(string orientation) {
    for (int i = 0; i < orientation.length(); i++) {
        orientation.at(i) = tolower(orientation.at(i));
    }
    if (orientation == "h" || orientation == "horizontal") {
        return HORIZONTAL;
    }
    if (orientation == "v" || orientation == "vertical") {
        return VERTICAL;
    }
    return -1;
}

int rowOfCoord(string coord) {
    if (!isValidCoord(coord)) {
        return -1;
    }
    
    char c = tolower(coord.at(0));
    return c - 96;
}

int colOfCoord(string coord) {
    if (!isValidCoord(coord)) {
        return -1;
    }
    
    if (coord.length() == 2) {
        return coord.at(1) - 48;
    }
    return (coord.at(1) - 48)*10 + (coord.at(2) - 48);
}

char getLetter(int n) {
    switch (n) {
        case 1: return 'A';
        case 2: return 'B';
        case 3: return 'C';
        case 4: return 'D';
        case 5: return 'E';
        case 6: return 'F';
        case 7: return 'G';
        case 8: return 'H';
        case 9: return 'I';
        case 10: return 'J';
        case 11: return 'K';
        case 12: return 'L';
        case 13: return 'M';
        case 14: return 'N';
        case 15: return 'O';
        case 16: return 'P';
        case 17: return 'Q';
        case 18: return 'R';
        case 19: return 'S';
        case 20: return 'T';
        case 21: return 'U';
        case 22: return 'V';
        case 23: return 'W';
        case 24: return 'X';
        case 25: return 'Y';
        case 26: return 'Z';
    }
    return '#';
}

void printNums() {
    for (int i = 0; i < MAXCOLS; i++) {
        if (i + 1 > 9) {
            cout << i + 1;
        }
        else {
            cout << i + 1 << " ";
        }
    }
}

void display(Grid* player, Grid* opponent) {
    cout << endl << "You:";
    for (int i = 0; i < MAXCOLS - 1; i++) {
        cout << "  ";
    }
    for (int i = 0; i < SPACE; i++) {
        cout << " ";
    }
    cout << "Opponent:" << endl << endl;
    
    cout << "  ";
    printNums();
    for (int i = 0; i < SPACE; i++) {
        cout << " ";
    }
    cout << "  ";
    printNums();
    cout << endl;
    
    for (int i = 0; i < MAXROWS; i++) {
        cout << getLetter(i + 1) << " ";
        for (int j = 0; j < MAXCOLS; j++) {
            if (player->getCellStatus(i + 1, j + 1) == EMPTY) {
                cout << ". ";
            }
            else if (player->getCellStatus(i + 1, j + 1) == OCCUPIED) {
                cout << "O ";
            }
            else if (player->getCellStatus(i + 1, j + 1) == HIT) {
                cout << "  ";
            }
            else {
                cout << "X ";
            }
        }
        for (int k = 0; k < SPACE; k++) {
            cout << " ";
        }
        cout << getLetter(i + 1) << " ";
        for (int j = 0; j < MAXCOLS; j++) {
            if (opponent->getCellStatus(i + 1, j + 1) == DAMAGED) {
                cout << "X ";
            }
            else if (opponent->getCellStatus(i + 1, j + 1) == HIT) {
                cout << "  ";
            }
            else {
                cout << ". ";
            }
        }
        cout << endl;
    }
}

void addOppShips(Grid* g) {
    string shipName;
    int length = 0;
    string coord;
    int orientation = 0;
    
    for (int i = 0; i < MAXSHIPS; i++) {
        switch (g->numShips()) {
            case 0: shipName = "Destroyer"; length = 2; break;
            case 1: shipName = "Submarine"; length = 3; break;
            case 2: shipName = "Cruiser"; length = 3; break;
            case 3: shipName = "Battleship"; length = 4; break;
            case 4: shipName = "Carrier"; length = 5; break;
            default: shipName = "Unnamed Ship"; length = 3; break;
        }
        do {
            int row = randInt(0, MAXROWS - 1);
            int col = randInt(0, MAXCOLS - 1);
            orientation = randInt(0, 1);
            coord = "";
            coord += getLetter(row + 1);
            coord += to_string(col + 1);
            //cout << "coord: " << coord << " " << orientation << endl;
        } while (!isValidShipPos(g, coord, length, orientation));
        //cout << "coord: " << coord << endl;
        g->addShip(shipName, coord, orientation, length);
    }
}

void attack(Grid* g, int row, int col) {
    if (g->getCellStatus(row, col) == OCCUPIED) {
        g->setCellStatus(row, col, DAMAGED);
        Ship* ship = g->hitShip(row, col);
        if (ship == nullptr) {
            cout << "Cell is labeled 'occupied' without any ship occupying it." << endl;
        }
        else {
            if (g->isOpponent() == false) {
                cout << "Enemy fired at " << getLetter(row) << col << " and struck your " << ship->name() << "." << endl;
                if (ship->hasSunk()) {
                    cout << "The enemy sank your " << ship->name() << "." << endl;
                    g->sinkShip(ship);
                }
            }
            else {
                cout << "You fired at " << getLetter(row) << col << " and struck an enemy ship." << endl;
                if (ship->hasSunk()) {
                    cout << "You sank the enemy's " << ship->name() << "." << endl;
                    g->sinkShip(ship);
                }
            }
        }
    }
    else if (g->getCellStatus(row, col) == EMPTY) {
        g->setCellStatus(row, col, HIT);
        if (g->isOpponent() == true) {
            cout << "You fired at " << getLetter(row) << col << " and struck water." << endl;
        }
        else {
            cout << "Enemy fired at " << getLetter(row) << col << " and struck water." << endl;
        }
    }
    else {
        cout << "You fired at a location that has been previously targeted. Such a waste of ammunition..." << endl;
    }
}




int main() {
    cout << "-----------------------------" << endl << "*** WELCOME TO BATTLESHIP ***" << endl << "-----------------------------" << endl << endl;
    Grid* player = new Grid(false);
    
    string coord = "";
    string orientation = "";
    int numOrient = -1;
    string shipName;
    int length = 0;
    bool valid = false;
    
    player->displayGrid();
    cout << endl;
    
    do {
        switch (player->numShips()) {
            case 0: shipName = "Destroyer"; length = 2; break;
            case 1: shipName = "Submarine"; length = 3; break;
            case 2: shipName = "Cruiser"; length = 3; break;
            case 3: shipName = "Battleship"; length = 4; break;
            case 4: shipName = "Carrier"; length = 5; break;
            default: shipName = "Unnamed Ship"; length = 3; break;
        }
        do {
            do {
                cout << "Select coordinates and orientation for your " << shipName << endl << "(length of ship: " << length << " units)" << endl << "Coordinates: ";
                getline(cin, coord);
                
                if (!isValidCoord(coord)) {
                    cout << endl << "***** Coordinates are invalid, please try again." << endl;
                }
            } while (!isValidCoord(coord));
            
            do {
                cout << "Select orientation (horizontal or vertical): ";
                getline(cin, orientation);
                cout << endl;
                numOrient = decodeOrientation(orientation);
                
                if (numOrient == -1) {
                    cout << "***** Orientation is invalid, please try again." << endl << endl;
                }
            } while (numOrient == -1);
            
            if (!isValidShipPos(player, coord, length, numOrient)) {
                cout << endl << "***** Ship cannot be placed because it either runs off the grid or is in the vicinity of another ship, please try again." << endl << endl;
                valid = false;
            }
            else {
                player->addShip(shipName, coord, numOrient, length);
                valid = true;
            }
            player->displayGrid();
            cout << endl;
            
        } while (!valid);
        
    } while (player->numShips() < MAXSHIPS);
    
    Grid* opponent = new Grid(true);
    addOppShips(opponent);
    
    int row = 0;
    int col = 0;
    
    bool findDirection = false;
    int setDirection = -1;
    int dir = -1;
    
    //starting round...
    while (player->numShips() > 0 && opponent->numShips() > 0) {
        display(player, opponent);
        cout << endl;
        cout << "Enter coordinates: ";
        getline(cin, coord);
        cout << endl;
        if (!isValidCoord(coord)) {
            cout << "You fired at " << coord << ", a location that is not even on the map. Such a waste of ammunition..." << endl;
        }
        else {
            attack(opponent, rowOfCoord(coord), colOfCoord(coord));
        }
        
        //opponent's turn
        string randCoord;
        
        int tempRow = row;
        int tempCol = col;
        
        if (!findDirection && setDirection == -1) {
            do {
                row = randInt(1, MAXCOLS);
                col = randInt(1, MAXCOLS);
                randCoord = getLetter(row) + to_string(col);
            } while (!isValidCoord(randCoord) || player->getCellStatus(row, col) == HIT || player->getCellStatus(row, col) == DAMAGED);
        }
        else if (setDirection != -1) {
            do {
                switch (setDirection) {
                    case NORTH: row--; break;
                    case EAST: col++; break;
                    case SOUTH: row++; break;
                    case WEST: col--; break;
                    default: cout << "Problem when setDirection != -1" << endl; break;
                }
                randCoord = getLetter(row) + to_string(col);
                
                if (!isValidCoord(randCoord) || player->getCellStatus(row, col) == HIT) {
                    switch (setDirection) {
                        case NORTH: setDirection = SOUTH; row++; break;
                        case EAST: setDirection = WEST; col--; break;
                        case SOUTH: setDirection = NORTH; row--; break;
                        case WEST: setDirection = EAST; col++; break;
                        default: cout << "Problem when not valid and hit" << endl; break;
                    }
                }
                 
            } while (!isValidCoord(randCoord) || player->getCellStatus(row, col) == HIT || player->getCellStatus(row, col) == DAMAGED);
        }
        else if (findDirection){
            do {
                int n = randInt(1, 4);
                row = tempRow;
                col = tempCol;
                
                switch (n) {
                    case 1: dir = NORTH; row--; break;
                    case 2: dir = EAST; col++; break;
                    case 3: dir = SOUTH; row++; break;
                    case 4: dir = WEST; col--; break;
                    default: dir = NORTH; break;
                }
                randCoord = getLetter(row) + to_string(col);
            } while (!isValidCoord(randCoord) || player->getCellStatus(row, col) == HIT || player->getCellStatus(row, col) == DAMAGED);
        }
        else {
            cout << "There is a problem here..." << endl;
        }
        
        int numShips = player->numShips();
        attack(player, row, col);
        
        if (player->getCellStatus(row, col) == DAMAGED && findDirection) {
            findDirection = false;
            switch (dir) {
                case NORTH: setDirection = NORTH; break;
                case EAST: setDirection = EAST; break;
                case SOUTH: setDirection = SOUTH; break;
                case WEST: setDirection = WEST; break;
                default: cout << "Issue with 'DAMAGED' and 'findDirection'" << endl; break;
            }
        }
        else if (player->getCellStatus(row, col) == DAMAGED) {
            findDirection = true;
        }
        else if (player->getCellStatus(row, col) != DAMAGED && findDirection) {
            switch (dir) {
                case NORTH: row++; break;
                case EAST: col--; break;
                case SOUTH: row--; break;
                case WEST: col++; break;
                default: cout << "Issue with !'DAMAGED' and 'findDirection'" << endl; break;
            }
        }
        else if (player->getCellStatus(row, col) == HIT && setDirection != -1) {
            switch (setDirection) {
                case NORTH: setDirection = SOUTH; break;
                case EAST: setDirection = WEST; break;
                case SOUTH: setDirection = NORTH; break;
                case WEST: setDirection = EAST; break;
                default: cout << "Issue with 'HIT' and 'setDirection'" << endl; break;
            }
        }
        
        if (player->numShips() != numShips) {
            findDirection = false;
            setDirection = -1;
        }
    }
    display(player, opponent);
    cout << endl << "-----------------------------" << endl;
    
    if (player->numShips() == 0 && opponent->numShips() == 0) {
        cout << "       ***  DRAW  ***       ";
        cout << endl << "-----------------------------" << endl;
        cout << "You and your opponent have destroyed each other. Neither are victorious." << endl;
    }
    else if (player->numShips() == 0) {
        cout << "      ***  DEFEAT  ***      ";
        cout << endl << "-----------------------------" << endl;
        cout << "The enemy has defeated you. All your men have been lost and you are forced to surrender. Your punishment awaits..." << endl;
    }
    else if (opponent->numShips() == 0) {
        cout << "      ***  VICTORY  ***     ";
        cout << endl << "-----------------------------" << endl;
        cout << "You have defeated the enemy and claim victory in this battle. But just because this battle is over does not mean the war has been won..." << endl;
    }
    else {
        cout << "I don't know what happened so I declare all your ships to have mysteriously disappeared, don't do whatever you just did to cause this message to be printed." << endl;
    }
}
