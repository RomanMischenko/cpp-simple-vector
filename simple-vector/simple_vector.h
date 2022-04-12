#pragma once

#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <iostream>
#include <algorithm>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) 
    : new_capacity_(capacity)
    {}
    size_t new_capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept 
    : items_(nullptr)
    , size_(0)
    , capacity_(0)
    {}

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        if (size == 0) {
            SimpleVector();
        } else {
            ArrayPtr<Type> arr(size);
            size_ = size;
            capacity_ = size;
            const auto& it = arr.Get();
            for (size_t i = 0; i < size; ++i) {
                *(it + i) = Type();
            }
            items_ = arr.Release();
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
    : SimpleVector<Type>(size)
    {
        for (size_t i = 0; i < size; ++i) {
            *(items_ + i) = value;
        }
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
    : SimpleVector<Type>(init.size())
    {   
        size_t i = 0;
        for (auto it = std::begin(init); it != std::end(init); ++it) {
            *(items_ + i) = *it;
            ++i;
        }
    }

    // Конструктор копирования
    SimpleVector(const SimpleVector& other)
    : size_(other.size_)
    , capacity_(other.capacity_)
    {
        ArrayPtr<Type> arr(other.size_);
        const auto& it = arr.Get();
        for (size_t i = 0; i < other.size_; ++i) {
            *(it + i) = *(other.items_ + i);
        }
        items_ = arr.Release();
    }

    // Конструктор перемещения
    SimpleVector(SimpleVector&& other)
    : items_(other.items_)
    , size_(other.size_)
    , capacity_(other.capacity_)
    {
        other.items_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Конструктор из функции Reserve
    SimpleVector(const ReserveProxyObj& obb) 
    : items_(nullptr)
    , size_(0)
    , capacity_(0)
    {
        Reserve(obb.new_capacity_);
    }

    ~SimpleVector() {
        delete[] items_;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector<Type> tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            SimpleVector<Type> tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        } else {
            return items_[index];
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out_of_range");
        } else {
            return items_[index];
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else if (new_size < capacity_) {
            for (size_t i = size_; i < new_size; ++i) {
                *(items_ + i) = Type();
            }
            size_ = new_size;
        } else {
            ArrayPtr<Type> arr(
                (size_ * 2 <= new_size) ? new_size : size_ * 2
            ); 
            const auto& it = arr.Get();
            for (size_t i = 0; i < size_; ++i) {
                *(it + i) = std::move(*(items_ + i));
            }
            for (size_t i = size_; i < new_size; ++i) {
                *(it + i) = Type();
            }
            items_ = arr.Release();
            size_ = new_size;
            capacity_ = new_size;
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            ArrayPtr<Type> arr(size_ * 2 + 1);
            std::copy(begin(), end(), arr.Get());
            *(arr.Get() + size_) = item;
            items_ = arr.Release();
            capacity_ = size_ * 2 + 1;
            ++size_;
        } else {
            *(items_ + size_) = item;
            ++size_;
        }
    }

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            ArrayPtr<Type> arr(size_ * 2 + 1);
            std::move(begin(), end(), arr.Get());
            *(arr.Get() + size_) = std::move(item);
            items_ = arr.Release();
            capacity_ = size_ * 2 + 1;
            ++size_;
        } else {
            *(items_ + size_) = std::move(item);
            ++size_;
        }
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ != 0) {
            --size_;
        }
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        if (size_ == capacity_) {
            SimpleVector<Type> tmp(size_ * 2 + 1);
            tmp.size_ = size_ + 1;
            auto it = std::copy(begin(), const_cast<Type*>(pos), tmp.begin());
            std::copy_backward(const_cast<Type*>(pos), end(), tmp.end());
            *it = value;
            swap(tmp);
            return it;
        } else {
            auto it = std::copy_backward(const_cast<Type*>(pos), end(), end() + 1);
            *(it - 1) = value;
            ++size_;
            return (it - 1);
        }       
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        if (size_ == capacity_) {
            SimpleVector<Type> tmp(size_ * 2 + 1);
            tmp.size_ = size_ + 1;
            auto it = std::move(begin(), const_cast<Type*>(pos), tmp.begin());
            std::move_backward(const_cast<Type*>(pos), end(), tmp.end());
            *it = std::move(value);
            swap(tmp);
            return it;
        } else {
            auto it = std::move_backward(const_cast<Type*>(pos), end(), end() + 1);
            *(it - 1) = std::move(value);
            ++size_;
            return (it - 1);
        }       
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        Iterator it = const_cast<Iterator>(pos);
        std::move(const_cast<Type*>(pos + 1), const_cast<Type*>(end()), const_cast<Type*>(pos));
        --size_;
        return it;
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        std::swap(items_, other.items_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> arr(new_capacity);
            std::copy(begin(), end(), arr.Get());
            items_ = arr.Release();
            capacity_ = new_capacity;
        }
    }
 
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_ + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_ + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_;
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_ + size_;
    } 
private:
    Type* items_;

    size_t size_;
    size_t capacity_;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
    );
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::equal(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
    );
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(
        lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
    );
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}
 
template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}
 
template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}