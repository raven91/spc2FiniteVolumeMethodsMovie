//
// Created by Nikita Kruk on 16.07.18.
//

#ifndef SPC2FVMNONLINNONLOCEQSMOVIE_SIMULATIONMODEL_HPP
#define SPC2FVMNONLINNONLOCEQSMOVIE_SIMULATIONMODEL_HPP

#include "Definitions.hpp"
#include "ParameterContainer.hpp"

#include <GLEW/glew.h>

#include <fstream>
#include <vector>
#include <map>

class SimulationModel
{
 public:

  SimulationModel(ParameterContainer *parameter_container);
  ~SimulationModel();

  void ReadNewState();
  void SkipTimeUnits(int t, int delta_t_recip = 1);
  void FindMinmaxDensity(const std::vector<Real> &density);

  GLfloat GetCurrentTime();
  const std::vector<GLfloat> &GetCurrentState();
  const std::map<int, std::vector<GLfloat>> &GetColors();
  const std::vector<int> &GetIndexes();
  Real GetMinDensity();
  Real GetMaxDensity();

 private:

  ParameterContainer *parameter_container_;
  GLfloat t_;
  std::vector<GLfloat> system_state_;
  std::map<int, std::vector<GLfloat>> colors_;
  std::vector<int> indexes_;
  int number_of_cells_;
  int number_of_state_variables_;
  std::ifstream simulation_file_;
  Real min_density_;
  Real max_density_;

  void ThreeDimIdxToOneDimIdx(int x, int y, int phi, int &idx);
  void OneDimIdxToThreeDimIdx(int idx, int &x, int &y, int &phi);
  template<typename T>
  T NormalizedValue(T value, T min, T max);

};

#endif //SPC2FVMNONLINNONLOCEQSMOVIE_SIMULATIONMODEL_HPP
