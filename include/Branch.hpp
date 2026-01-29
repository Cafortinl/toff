#ifndef BRANCH_HPP
#define BRANCH_HPP

#include <string>

class Branch {
    private:
        int id;
        std::string name;

    public:
        Branch(std::string name);
        ~Branch();

        int getId();

        std::string getName();
        void setName(std::string name);
};
#endif
