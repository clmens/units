#include "Unit.h"
#include <iostream>

Unit::Unit(int _x, int _y, double max_value)
: x(_x), y(_y), delta(0.0), delta_step(0.0), pushed_delta(0.0), m_value(0.0), m_target(0.0), m_max_value(max_value)
{
}

void Unit::reset()
{
    delta = 0.0;
    delta_step = 0.0;
}

void Unit::set_value(double value)
{
    m_value = value;
}

void Unit::update()
{
    if(m_value > m_max_value) m_value = m_max_value;
    if(m_value < -m_max_value) m_value = -m_max_value;
    
    m_value += delta_step + delta;
    
    delta = (m_target - m_value);
    delta_step = 0.0;
    
    //push();
    
}

void Unit::push()
{
    double delta_con(delta/connections.size());
    
    for (auto connection: connections)
    {
        connection-> delta_step += -delta_con;
         
    }
    
}

std::ostream& Unit::print(std::ostream& os) const
{
    os<<" # unit ("<<x<<","<<y<<"): "<<m_value<<" delta: "<<delta<<" step: "<<delta_step;
    return os;
}

std::ostream& Unit::print_connections(std::ostream& os) const
{
    for (auto con: connections)
    {
        con->print(os);
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Unit const& u)
{
    u.print(os);
    //u.print_connections(os);
    return os;
}
