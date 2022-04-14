#pragma once
#include <cassert>
#include <stdexcept>
#include <initializer_list>
#include <algorithm>
#include <utility>
#include <iterator>
#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) 
        : capacity_(capacity)
    {
        
    }

    size_t GetCapacity() const {
        return capacity_;
    }
private:
    size_t capacity_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : items_(ArrayPtr<Type>(size))
        , size_(size)
        , capacity_(size) // ???
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
        : SimpleVector(size)// Invoke delegating constructor
    {
        std::fill(items_.Get(), items_.Get() + size_, value);
    }

    //Конструктор из std::initializer_list
    SimpleVector(const std::initializer_list<Type> list) 
        : SimpleVector(list.size())// Invoke delegating constructor
    {
        std::copy(list.begin(), list.end(), items_.Get());
    }

    //copying constructor
    SimpleVector(const SimpleVector& other) 
        : items_(ArrayPtr<Type>(other.GetCapacity()))
        , size_(other.GetSize())
        , capacity_(other.GetCapacity())
    {
        std::copy(other.begin(), other.end(), begin());
    }

    //reseve method constructor
    SimpleVector(ReserveProxyObj object) {
        Reserve(object.GetCapacity());
    }

    //movable obj constructor
    SimpleVector(SimpleVector&& other) 
    {
        items_ = std::exchange(other.items_, ArrayPtr<Type>());
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    //movable assignment
    // SimpleVector& operator=(SimpleVector&& rhs) 
    //     : items_(std::move(rhs.items_))
    //     , size_(rhs.GetSize())
    //     , capacity(rhs.GetCapacity())
    // {
    //     return *this;
    // }

    SimpleVector& operator=(SimpleVector&& rhs) = default;

    //Named obj movable constructor
    // SimpleVector(SimpleVector other) 
    //     : items_(std::move(other.items_))
    //     , size_(other.GetSize())
    //     , capacity_(other.GetCapacity())
    // {
    // }

    //Named obj movable assignment
    // SimpleVector& operator=(SimpleVector rhs) {
    //     if(*this != rhs) {
    //         std::
    //     }
    // }

    // ~SimpleVector() {
    //     delete[] items_.Get();
    // }

    void Resize(size_t new_size) noexcept;
    void Clear() noexcept;
    size_t GetSize() const noexcept;
    size_t GetCapacity() const noexcept;
    bool IsEmpty() const noexcept;
 
    
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if(size_ == capacity_) {
            size_t new_capacity = 1;
            capacity_ > 0 ? new_capacity = 2 * capacity_ : new_capacity = 1;
            SimpleVector doubled_vector(new_capacity);
            doubled_vector.size_ = size_;
            std::copy(begin(), end(), doubled_vector.begin());
            swap(doubled_vector);
        }
        Iterator end_pos = end();
        *end_pos = item;
        ++size_;
    }

    //movable, not copy
    void PushBack(Type&& item) {
        if(size_ == capacity_) {
            size_t new_capacity = 1;
            capacity_ > 0 ? new_capacity = 2 * capacity_ : new_capacity = 1;
            SimpleVector doubled_vector(new_capacity);
            doubled_vector.size_ = size_;
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), doubled_vector.begin());
            swap(doubled_vector);
        }
        Iterator end_pos = end();
        *end_pos = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        if(size_ < capacity_) {
             
            std::copy_backward(pos, cend(), end() + 1); 
            Iterator pos_iterator = const_cast<Iterator>(pos);
            *pos_iterator = value;
            ++size_;
            return pos_iterator;
        } else {
            size_t new_capacity = 1;
            capacity_ > 0 ? new_capacity = 2 * capacity_ : new_capacity = 1;
            SimpleVector doubled_vector(new_capacity);
            
            std::copy(cbegin(), pos, doubled_vector.begin());
            doubled_vector.size_ = std::distance(cbegin(), pos);
            
            Iterator pos_iterator = doubled_vector.end();
            doubled_vector.PushBack(value);
            
            std::copy(pos, cend(), doubled_vector.end());
            
            doubled_vector.size_ = size_ + 1;
            swap(doubled_vector);
            return pos_iterator;
        }
    }

    //movable, not copy
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        if(size_ < capacity_) {
            Iterator nonconst_pos = const_cast<Iterator>(pos);
            std::copy_backward(std::make_move_iterator(nonconst_pos), std::make_move_iterator(end()), std::make_move_iterator(end() + 1)); 
            
            *nonconst_pos = std::move(value);
            ++size_;
            return nonconst_pos;
        } else {
            size_t new_capacity = 1;
            capacity_ > 0 ? new_capacity = 2 * capacity_ : new_capacity = 1;
            SimpleVector doubled_vector(new_capacity);
            
            Iterator nonconst_pos = const_cast<Iterator>(pos);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(nonconst_pos), doubled_vector.begin());
            doubled_vector.size_ = std::distance(cbegin(), pos);
            
            Iterator pos_iterator = doubled_vector.end();
            //doubled_vector.PushBack(value);
            *pos_iterator = std::move(value);
            
            std::copy(std::make_move_iterator(nonconst_pos), std::make_move_iterator(end()), doubled_vector.end());
            
            doubled_vector.size_ = size_ + 1;
            swap(doubled_vector);
            return pos_iterator;
        }
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if(!IsEmpty()) {
            --size_;
        }
    }

    //Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        Iterator to_erase = const_cast<Iterator>(pos);
        Iterator next_to_erase = const_cast<Iterator>(pos + 1);
        Iterator end_iterator = end();

        std::copy(std::make_move_iterator(next_to_erase), std::make_move_iterator(end_iterator), to_erase);
        --size_;
        return Iterator(const_cast<Iterator>(pos));
    }
    
    //задает ёмкость вектора
    void Reserve(size_t new_capacity) {
        if(capacity_ < new_capacity) {
            SimpleVector reserved_vector(new_capacity);
            std::copy(begin(), end(), reserved_vector.begin());
            reserved_vector.size_ = size_;
            swap(reserved_vector);
        }
    }

    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return *(items_.Get() + index);
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return *(items_.Get() + index);
    }

    Type& At(size_t index) {
        if(index >= size_) {
            throw std::out_of_range("big index!");
        }
        else {
            return *(items_.Get() + index);
        }
    }

    const Type& At(size_t index) const {
        if(index >= size_) {
            throw std::out_of_range("big index!");
        }
        else {
            return *(items_.Get() + index);
        }
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return begin();
    }

    ConstIterator cend() const noexcept {
        return end();
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    SimpleVector& operator=(const SimpleVector& rhs) noexcept {
        if(*this != rhs) {
            SimpleVector rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if(lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.cbegin(), rhs.cend(), lhs.cbegin(), lhs.cend());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
void SimpleVector<Type>::Resize(size_t new_size) noexcept {
    if(new_size > capacity_) {
        ArrayPtr<Type> new_items_(new_size);
        std::copy(std::make_move_iterator(items_.Get()), std::make_move_iterator(items_.Get() + size_), new_items_.Get());

        ArrayPtr<Type> empty_items(new_size - size_);
        std::copy(std::make_move_iterator(empty_items.Get()), std::make_move_iterator(empty_items.Get() + new_size - size_), new_items_.Get() + size_);
        //std::fill(std::make_move_iterator(new_items_.Get() + size_), std::make_move_iterator(new_items_.Get() + new_size), std::move(Type()));
        items_.swap(new_items_);
        size_ = new_size;
        capacity_ = new_size;
    } else if(new_size > size_ && new_size <= capacity_) {

        ArrayPtr<Type> empty_items(new_size - size_);
        std::copy(std::make_move_iterator(empty_items.Get()), std::make_move_iterator(empty_items.Get() + new_size - size_), items_.Get() + size_);
        
        size_ = new_size;
    } else {
        size_ = new_size;
    }
}

template <typename Type>
void SimpleVector<Type>::Clear() noexcept {
    size_ = 0;
}

template <typename Type>
size_t SimpleVector<Type>::GetSize() const noexcept {
    return size_;
}

template <typename Type>
size_t SimpleVector<Type>::GetCapacity() const noexcept {
    return capacity_;
}

template <typename Type>
bool SimpleVector<Type>::IsEmpty() const noexcept {
    return size_ == 0 ? true : false;
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}