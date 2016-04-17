/*
 * TargetPhrases.h
 *
 *  Created on: 23 Oct 2015
 *      Author: hieu
 */

#pragma once
#include <vector>
#include "../Vector.h"

namespace Moses2
{
class TargetPhrase;

class TargetPhrases
{
  friend std::ostream& operator<<(std::ostream &, const TargetPhrases &);
  typedef Array<const TargetPhrase*> Coll;
public:
  typedef Coll::iterator iterator;
  typedef Coll::const_iterator const_iterator;
  //! iterators
  const_iterator begin() const
  {
    return m_coll.begin();
  }
  const_iterator end() const
  {
    return m_coll.end();
  }

  TargetPhrases(MemPool &pool, size_t size);
  //TargetPhrases(MemPool &pool, const System &system, const TargetPhrases &copy);
  virtual ~TargetPhrases();

  void AddTargetPhrase(const TargetPhrase &targetPhrase)
  {
    m_coll[m_currInd++] = &targetPhrase;
  }

  size_t GetSize() const
  {
    return m_coll.size();
  }

  const TargetPhrase& operator[](size_t ind) const
  {
    return *m_coll[ind];
  }

  void SortAndPrune(size_t tableLimit);

  //const TargetPhrases *Clone(MemPool &pool, const System &system) const;
protected:
  Coll m_coll;
  size_t m_currInd;
};

}
