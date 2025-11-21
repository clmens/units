#ifndef __NN__Unit__
#define __NN__Unit__

#include <iostream>
#include <memory>

class Unit
{
public:
    Unit(int , int , double );
    
    void set_value(double value);
    
    void reset();
    void update();
    void push();
    
    std::vector<std::shared_ptr<Unit>> connections;
    
    int x, y;
    double delta;
    double delta_step;
    double pushed_delta;

    double m_value;
    double m_target;
    double m_max_value;
    
    std::ostream& print(std::ostream& ) const;
    std::ostream& print_connections(std::ostream& ) const;

    
private:
};

std::ostream& operator <<(std::ostream& , Unit const& );


#endif /* defined(__NN__Unit__) */
