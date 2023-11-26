#pragma once
#include "rtweekend.h"

template<class T> 
class variance_welford{
public:
    variance_welford(T m_initial) : n(0), m_mean(m_initial), m_sum2(m_initial) {}

    variance_welford& operator+=(const T& rhs){
        add_sample(rhs);
        return *this;
    }

    T mean() const {
        return m_mean;
    }

    T variance() const {
        if(n <= 2)
          return T();
        return m_sum2 / static_cast<double>(n - 1);
    }

    T standard_deviation() const {
        return sqrt(variance());
    }
private:
    void add_sample(const T& x) {
        n++;
        T delta = x - m_mean;
        m_mean += delta / static_cast<double>(n);
        auto new_delta = (x - m_mean);
        m_sum2 += delta * new_delta;
    }

  T m_sum2, m_mean;
  int n;
}; 


template<class T> 
class weighted_variance_welford{
public:
    weighted_variance_welford(T m_initial) : weight_sum(0), m_mean(m_initial), m_sum2(m_initial) {}

    void add_sample(const T& x, double weight) {
        weight_sum += weight;
        T delta = x - m_mean;
        m_mean += (weight/weight_sum)*delta;
        T new_delta = x - m_mean;
        m_sum2 += weight * delta * new_delta;
    }

    T mean() const {
        return m_mean;
    }

    T variance() const {
        if(weight_sum <= 1)
          return T();
        return m_sum2 / (weight_sum - 1);
    }

    T standard_deviation() const {
        return sqrt(variance());
    }
private:
  T m_sum2, m_mean;
  double weight_sum;
}; 