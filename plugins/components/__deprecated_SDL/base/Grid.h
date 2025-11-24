/** Description:
This C++ header defines a utility class called `Grid`, designed specifically to manage selection and movement within a structured, two-dimensional layout, such as a menu or a graphical table interface.

**Core Functionality:**
The `Grid` acts like a sophisticated coordinate system manager. When initialized, it establishes the total size by defining the maximum number of rows and columns available.

**Tracking Movement:**
The class maintains two sets of coordinates: the *current* selected position and the *last* position visited. By comparing these two, the system can instantly determine if the selection has changed (if the user moved up, down, left, or right) since the previous interaction. This change detection is vital for updating a user interface efficiently.

**Interaction and Control:**
The system includes simple functions to move the selection: `up`, `down`, `left`, and `right`. There is also a function to jump directly to a specific spot. Crucially, the class ensures safety: if a movement command attempts to go past the defined boundaries (e.g., trying to move right past the last column), the internal logic automatically locks the selection to the valid edge, preventing errors or navigating outside the allowed area.

In essence, the `Grid` class provides a robust, contained system for navigating a predefined visual space, making it easy to build interfaces that respond accurately and safely to directional inputs.

sha: db8ff5eed0aee517965d67fc3b5bc24d8d17463ff7eee33fcd673335fb375cd3 
*/
#ifndef _UI_GRID_H_
#define _UI_GRID_H_

#include <stdint.h>

class Grid {
public:
    int8_t row = 0;
    int8_t col = 0;

    int8_t lastRow = 0;
    int8_t lastCol = 0;

    uint8_t rows = 0;
    uint8_t cols = 0;

    Grid(uint8_t rows, uint8_t cols, int8_t row = 0, int8_t col = 0)
    {
        this->rows = rows;
        this->cols = cols;
        this->row = row;
        this->col = col;
    }

    void reset()
    {
        lastRow = 0;
        lastCol = 0;
        row = 0;
        col = 0;
    }

    bool is(int8_t row, int8_t col)
    {
        return this->row == row && this->col == col;
    }

    bool lastIs(int8_t row, int8_t col)
    {
        return this->lastRow == row && this->lastCol == col;
    }

    bool rowChanged()
    {
        return row != lastRow;
    }

    bool colChanged()
    {
        return col != lastCol;
    }

    void up()
    {
        selectNextRow(-1);
    }

    void down()
    {
        selectNextRow(+1);
    }

    void left()
    {
        selectNextCol(-1);
    }

    void right()
    {
        selectNextCol(+1);
    }

    void select(int8_t _row, int8_t _col)
    {
        lastRow = row;
        lastCol = col;
        row = _row;
        col = _col;
    }

    void selectNextRow(int8_t dir)
    {
        lastRow = row;
        lastCol = col;
        row += dir;
        if (row >= rows) {
            row = rows - 1;
        } else if (row < 0) {
            row = 0;
        }
    }

    void selectNextCol(int8_t dir)
    {
        lastRow = row;
        lastCol = col;
        col += dir;
        if (col >= cols) {
            col = cols - 1;
        } else if (col < 0) {
            col = 0;
        }
    }
};

#endif