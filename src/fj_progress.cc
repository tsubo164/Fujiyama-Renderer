/*
Copyright (c) 2011-2014 Hiroshi Tsubokawa
See LICENSE and README
*/

#include "fj_progress.h"
#include <iostream>
#include <cassert>

namespace fj {

Progress::Progress() : total_iterations_(0), iteration_(0)
{
}

Progress::~Progress()
{
}

void Progress::Start(Iteration total_iterations)
{
  assert(total_iterations > 0);

  total_iterations_ = total_iterations;
  iteration_ = 0;
  std::cout << "....1....2....3....4....5....6....7....8....9....0\n";
}

ProgressStatus Progress::Increment()
{
  static const int TOTAL_OUTPUTS = 50;
  static const float OUTPUTS_DIV = 100./TOTAL_OUTPUTS;

  if (iteration_ >= total_iterations_) {
    std::cerr << "warning: progress incremented after reaching total iterations\n";
    std::cerr << std::endl;
  }

  const float prev_percent = iteration_ / (float)total_iterations_ * 100.;
  iteration_++;
  const float next_percent = iteration_ / (float)total_iterations_ * 100.;

  const int prev_outputs = (int) (prev_percent / OUTPUTS_DIV);
  const int next_outputs = (int) (next_percent / OUTPUTS_DIV);
  const int diff_outputs = next_outputs - prev_outputs;

  for (int i = 0; i < diff_outputs; i++) {
    std::cout << '-' << std::flush;
  }

  if (iteration_ == total_iterations_) {
    return PROGRESS_DONE;
  } else {
    return PROGRESS_ONGOING;
  }
}

void Progress::Done()
{
  if (iteration_ != total_iterations_) {
    std::cerr << "warning: progress done before reaching total iterations: ";
    std::cerr << iteration_ << "/" << total_iterations_ << '\n';
    std::cerr << std::flush;
  }

  std::cout << '\n';
}

} // namespace xxx
