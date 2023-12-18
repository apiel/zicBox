#ifndef _UI_GRID_H_
#define _UI_GRID_H_

#include <stdint.h>

class Grid {
protected:
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
};

#endif