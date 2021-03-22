//
// Created by Nikita Kruk on 16.07.18.
//

#include "SimulationModel.hpp"

#include <string>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <cmath>
#include <limits>

SimulationModel::SimulationModel(ParameterContainer *parameter_container) :
    parameter_container_(parameter_container),
    t_(0.0),
    system_state_(),
    colors_(),
    indexes_(),
    number_of_cells_(parameter_container->GetNumberOfCells()),
    number_of_state_variables_(parameter_container->GetNumberOfStateVariables())
{
  simulation_file_.open(parameter_container->GetSimulationFileName(), std::ios::binary | std::ios::in);
  assert(simulation_file_.is_open());
  system_state_.resize(number_of_state_variables_ * number_of_cells_, 0.0f);

  for (int i = 0; i < parameter_container_->GetN(); ++i)
  {
    for (int j = 0; j < parameter_container_->GetM(); ++j)
    {
      for (int k = 0; k < parameter_container_->GetL(); ++k)
      {
        int idx_cur = 0;
        ThreeDimIdxToOneDimIdx(i, j, k, idx_cur);
        system_state_[number_of_state_variables_ * idx_cur] =
            GLfloat((i + 0.5) / parameter_container_->GetN()); // x : left to right, [0,1)
        system_state_[number_of_state_variables_ * idx_cur + 1] = GLfloat(
            (parameter_container_->GetM() - j - 1 + 0.5) / parameter_container_->GetM());  // y : into the screen, [0,1)
        system_state_[number_of_state_variables_ * idx_cur + 2] =
            GLfloat((k + 0.5) / parameter_container_->GetL()); // phi : bottom to top, [0,1)
        system_state_[number_of_state_variables_ * idx_cur + 3] = 0.0;
      }
    }
  }
}

SimulationModel::~SimulationModel()
{
  simulation_file_.close();
}

void SimulationModel::ReadNewState()
{
  Real t = 0.0f;
  static std::vector<Real> density(number_of_cells_, 0.0);
  simulation_file_.read((char *) &t, sizeof(Real));
  t_ = t;
  simulation_file_.read((char *) &density[0], number_of_cells_ * sizeof(Real));
  std::cout << "total mass: "
            << std::accumulate(density.begin(), density.end(), 0.0) * parameter_container_->GetDifferentialVolume()
            << std::endl;
  /*for (int idx_cur = 0; idx_cur < density.size(); ++idx_cur)
  {
    int i = 0, j = 0, k = 0;
    OneDimIdxToThreeDimIdx(idx_cur, i, j, k);
    Real k1 = 4, k2 = 4, k3 = 4;
    density[idx_cur] = std::sin(2.0 * M_PI * k1 * i * parameter_container_->GetDx())
        * std::sin(2.0 * M_PI * k2 * j * parameter_container_->GetDy() - 2.0 * M_PI / 3.0)
        * std::sin(k3 * k * parameter_container_->GetDphi() - 4.0 * M_PI / 3.0);
  }*/

  FindMinmaxDensity(density);
  std::cout << "min:" << min_density_ << ", max:" << max_density_ << std::endl;

  for (int idx_cur = 0; idx_cur < density.size(); ++idx_cur)
  {
    system_state_[number_of_state_variables_ * idx_cur + 3] =
        NormalizedValue(density[idx_cur], min_density_, max_density_);
  }

  std::cout << "t:" << t << std::endl;
}

void SimulationModel::SkipTimeUnits(int t, int delta_t_recip)
{
  simulation_file_.seekg(int(t * delta_t_recip) * (1l + number_of_cells_) * sizeof(Real), std::ios::cur);
}

void SimulationModel::FindMinmaxDensity(const std::vector<Real> &density)
{
  std::pair<std::vector<Real>::const_iterator, std::vector<Real>::const_iterator>
      min_max = std::minmax_element(density.begin(), density.end());
  min_density_ = *min_max.first;
  max_density_ = *min_max.second;
}

GLfloat SimulationModel::GetCurrentTime()
{
  return t_;
}

const std::vector<GLfloat> &SimulationModel::GetCurrentState()
{
  return system_state_;
}

const std::map<int, std::vector<GLfloat>> &SimulationModel::GetColors()
{
  return colors_;
}

const std::vector<int> &SimulationModel::GetIndexes()
{
  return indexes_;
}

void SimulationModel::ThreeDimIdxToOneDimIdx(int x, int y, int phi, int &idx)
{
  // the winding order is x->y->phi
//    idx = x + kN * (y + kM * phi);
  // the winding order is phi->x->y
  idx = phi + parameter_container_->GetL() * (x + parameter_container_->GetN() * y);
}

void SimulationModel::OneDimIdxToThreeDimIdx(int idx, int &x, int &y, int &phi)
{
  // the winding order is x->y->phi
//    phi = idx / (kN * kM);
//    y = (idx % (kN * kM)) / kN;
//    x = idx % kN;
  // the winding order is phi->x->y
  y = idx / (parameter_container_->GetL() * parameter_container_->GetN());
  x = (idx % (parameter_container_->GetL() * parameter_container_->GetN())) / parameter_container_->GetL();
  phi = idx % parameter_container_->GetL();
}

template<typename T>
T SimulationModel::NormalizedValue(T value, T min, T max)
{
  return (value - min) / (max - min);
//  return value;
}