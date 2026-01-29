#ifndef POSITION_HPP
#define POSITION_HPP

#include <string>

class Position {
    private:
        int id;
        std::string name;

    public:
        Position(std::string name);
        ~Position();

        int getId();

        std::string getName();
        void setName(std::string name);
};
#endif
