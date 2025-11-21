#ifndef __NN__Net__
#define __NN__Net__

#include <iostream>
#include <memory>
#include <vector>

#include "Unit.h"

class Net
{
public:
    Net(int , double , double );
    
    void wire_quadratic();
    
    void update();
    
    
    void reset();
    void rand_unit_values();
    void same_values();

    void rand_unit_targets();
    void reset_targets();
    
    void print();
    
    std::vector<std::shared_ptr<Unit>> m_units;

    
private:
    int m_num_units;
    double m_value_range;
    double m_net_target;
};


#endif /* defined(__NN__Net__) */
