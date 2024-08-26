#ifndef ITERATORS_DOUBLE_ENDED_HPP
#define ITERATORS_DOUBLE_ENDED_HPP

namespace iter {

template <typename Self, typename T>
class DoubleEndedIterator {
public:
private:
    const Self& self() const { return *static_cast<const Self*>(this); }
    Self& self() { return *static_cast<Self*>(this); }
};

}  // namespace iter

#endif  // ITERATORS_DOUBLE_ENDED_HPP
