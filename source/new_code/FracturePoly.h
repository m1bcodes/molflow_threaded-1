#pragma once

#include "Clipper/clipper.hpp"

void processClipperSolution(const ClipperLib::PolyTree& solution, ClipperLib::Paths& paths);

