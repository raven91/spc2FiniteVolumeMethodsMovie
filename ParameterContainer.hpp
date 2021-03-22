//
// Created by Nikita Kruk on 16.07.18.
//

#ifndef SPC2FVMNONLINNONLOCEQSMOVIE_PARAMETERCONTAINER_HPP
#define SPC2FVMNONLINNONLOCEQSMOVIE_PARAMETERCONTAINER_HPP

#include "Definitions.hpp"

#include <GLEW/glew.h>

#include <string>

class ParameterContainer
{
 public:

  ParameterContainer();
  ~ParameterContainer();

  int GetN();
  int GetM();
  int GetL();
  int GetNumberOfCells();
  int GetNumberOfStateVariables();
  const std::string &GetSimulationFileName();
  int GetDtRecip();
  GLfloat GetDx();
  GLfloat GetDy();
  GLfloat GetDphi();
  GLfloat GetDifferentialVolume();

 private:

  std::string simulation_file_name_;
  int n_;
  int m_;
  int l_;
  int number_of_state_variables_;
  int dt_recip_;

  GLfloat x_size_;
  GLfloat y_size_;
  GLfloat phi_size_;
  GLfloat dx_;
  GLfloat dy_;
  GLfloat dphi_;

};

#endif //SPC2FVMNONLINNONLOCEQSMOVIE_PARAMETERCONTAINER_HPP
