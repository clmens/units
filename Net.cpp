#include "Net.h"
#include "cinder/Rand.h"


Net::Net(int num, double target, double range)
: m_num_units(num), m_net_target(target), m_value_range(range)
{
    cinder::Rand::randomize();
    
    for (int j = 0; j < m_num_units; ++j)
    {
        
        for(int i = 0; i < m_num_units; ++i)
        {
            std::shared_ptr<Unit> unit = std::make_shared<Unit>(i,j, range);
            unit->m_target = target;
            m_units.push_back(unit);
        }
    }
    
    wire_quadratic();
    
}

void Net::wire_quadratic()
{
    std::shared_ptr<Unit> first, last, top, bottom, prev, next, above, below;
    
    int all_units = m_num_units * m_num_units;
    
    for(int i = 0; i < all_units; ++i)
    {
        //normal cases
        if( i >= 1) prev = m_units[i-1];
        if( i <= (all_units-1)) next = m_units[i+1];
        
        if( (i-m_num_units) >= 0) above = m_units[i-m_num_units];
        if( (i+m_num_units) <= (all_units-1)) below = m_units[i+m_num_units];
        
        //border cases
        if (i % m_num_units == 0 ) prev = m_units[i+m_num_units-1];
        if (i % m_num_units == (m_num_units-1)) next = m_units[i-m_num_units+1];
        
        if ( (i - m_num_units) < 0 ) above = m_units[i+all_units-m_num_units];
        if ( (i + m_num_units) >= all_units ) below = m_units[i-all_units+m_num_units];
            
        
        m_units[i]->connections.push_back(prev);
        m_units[i]->connections.push_back(next);
        
        m_units[i]->connections.push_back(above);
        m_units[i]->connections.push_back(below);
    }
    
}

void Net::update()
{
    for (auto unit: m_units)
    {
        unit->update();
        //std::cout<<*unit<<std::endl;

    }
    
    for (auto unit: m_units)
    {
        unit->push();
    }
}

void Net::reset()
{
    for (auto unit: m_units)
    {
        unit->reset();
    }
}

void Net::rand_unit_values()
{
    for (auto unit: m_units)
    {
        cinder::Rand::randomize();
        //unit->m_value = cinder::randFloat(-m_value_range, m_value_range);
        //unit->m_value = cinder::randGaussian();
        unit->m_value = cinder::lmap<float>(cinder::randGaussian(), -3.0, 3.0, -m_value_range, m_value_range);
        
        //std::cout<<*unit<<std::endl;
    }
    
}

void Net::same_values()
{
    for (auto unit: m_units)
    {
        unit -> reset();
        unit->m_value = m_value_range;
    }
}

void Net::rand_unit_targets()
{
    for (auto unit: m_units)
    {
        cinder::Rand::randomize();
        //unit->m_target = cinder::randFloat(-m_value_range, m_value_range);
        unit->m_target = cinder::lmap<float>(cinder::randGaussian(), -3.0, 3.0, -m_value_range, m_value_range);
    }
}

void Net::reset_targets()
{
    for (auto unit: m_units)
    {
        unit->m_target = m_net_target;
    }
}

void Net::print()
{
    double net_value(0.0);
    double net_delta(0.0);
    double net_target(0.0);
    
    for ( auto unit: m_units)
    {
        net_value += unit->m_value;
        net_target += unit->m_target;
        net_delta += unit->delta;
    }
    std::cout<<"Overall Net value: "<<net_value<<" per Unit: "<<net_value/m_units.size()<<" target: "<<net_target/m_units.size()<<" Net delta: "<<net_delta<<std::endl;
}