#include "Stacks.h"
#include "Stack.h"

namespace Moses2
{

namespace SCFG
{

void Stacks::Init(SCFG::Manager &mgr, size_t size)
{
  m_cells.resize(size);
  for (size_t startPos = 0; startPos < size; ++startPos) {
    std::vector<Stack*> &inner = m_cells[startPos];
    inner.reserve(size - startPos);
    for (size_t endPos = startPos; endPos < size; ++endPos) {
      inner.push_back(new Stack(mgr));
    }
  }
}

}
}