//
// Created by Nikita Kruk on 16.07.18.
//

#include "ParameterContainer.hpp"

#include <cmath>

ParameterContainer::ParameterContainer() :
    n_(40),
    m_(40),
    l_(256),
    number_of_state_variables_(4),
    dt_recip_(1),
    x_size_(1.0f),
    y_size_(1.0f),
    phi_size_(2.0f * M_PI),
    dx_(x_size_ / n_),
    dy_(y_size_ / m_),
    dphi_(phi_size_ / l_)
{
//  std::string
//      folder("/Users/nikita/Documents/Projects/spc2/spc2FiniteVolumeMethods/");
  std::string folder("/Volumes/Kruk/spc2/spc2FiniteVolumeMethods/Continuation/ContinuationPhaseLagFromNonhomogeneous/left_path/");
  simulation_file_name_ =
      folder + "dt_0.005_sigma_1_rho_0.3_alpha_1.44_Dphi_0.0075_" + std::to_string(n_) + "_" + std::to_string(m_) + "_"
          + std::to_string(l_) + "_1000.bin";
}

ParameterContainer::~ParameterContainer()
{

}

int ParameterContainer::GetN()
{
  return n_;
}

int ParameterContainer::GetM()
{
  return m_;
}

int ParameterContainer::GetL()
{
  return l_;
}

int ParameterContainer::GetNumberOfCells()
{
  return n_ * m_ * l_;
}

int ParameterContainer::GetNumberOfStateVariables()
{
  return number_of_state_variables_;
}

const std::string &ParameterContainer::GetSimulationFileName()
{
  return simulation_file_name_;
}

int ParameterContainer::GetDtRecip()
{
  return dt_recip_;
}

GLfloat ParameterContainer::GetDx()
{
  return dx_;
}

GLfloat ParameterContainer::GetDy()
{
  return dy_;
}

GLfloat ParameterContainer::GetDphi()
{
  return dphi_;
}

GLfloat ParameterContainer::GetDifferentialVolume()
{
  return dx_ * dy_ * dphi_;
}