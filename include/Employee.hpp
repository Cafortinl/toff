#ifndef EMPLOYEE_HPP
#define EMPLOYEE_HPP

#include <string>

#include "Branch.hpp"
#include "Department.hpp"
#include "Position.hpp"

class Employee {
    private:
        bool active;
        int id, dayBalance;
        std::string name;
        Branch branch;
        Department department;
        Position position;

    public:
        Employee (
            int id,
            std::string name,
            Branch branch,
            Department department,
            Position position,
            int dayBalance,
            bool active 
        );
        ~Employee();

        int getId();
        void setId(int id);

        std::string getName();
        void setName(std::string name);

        Position getPosition();
        void setPosition(Position position);

        Branch getBranch();
        void setBranch(Branch branch);

        Department getDepartment();
        void setDepartment(Department department);

        bool isActive();
        void setActive(bool active);

        int getDayBalance();
        void setDayBalance(int dayBalance);
};
#endif
