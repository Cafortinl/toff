#ifndef DEPARTMENT_HPP
#define DEPARTMENT_HPP

#include <string>

class Department {
    private:
        int id;
        std::string name;

    public:
        Department(std::string name);
        ~Department();

        int getId();

        std::string getName();
        void setName(std::string name);
};
#endif
