#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <generator>
#include <utility>

namespace mynum::impl {

std::generator<int> submasks(int mask);

std::generator<std::pair<int, int>> submask_complement_pairs(int mask, bool symmetry = false);

}

#endif
