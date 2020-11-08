#ifndef RECAP_CUDA_ASSIGNMENT_ALGORITHM_HPP_
#define RECAP_CUDA_ASSIGNMENT_ALGORITHM_HPP_

#include <iostream>
#include <cstdint>
#include <vector>
#include <cstdint>

#include <cuda_runtime.h>

#include "recipe.hpp"
#include "resistance.hpp"
#include "assignment.hpp"
#include "assignment_kernel.hpp"
#include "parallel_assignment_algorithm.hpp"

namespace recap
{
    /** Check return value of a cuda API function
     * 
     * @param result Return value of a cuda API function
     * @param line Program line number
     * @param file Program source file path
     */
    inline void check_cuda_result(cudaError_t result, int line, const char* file)
    {
        if (result != cudaSuccess)
        {
            std::cerr << "CUDA Error at " << file << ":" << line << " - " << cudaGetErrorString(result) << std::endl;
        }
    }

    #define CUCHECK(result) ::recap::check_cuda_result((result), __LINE__, __FILE__)

    /** Pointer to memory on the device (GPU)
     * 
     * @tparam T type of one element of the array pointed to by this class
     */
    template<typename T>
    class gpu_ptr
    {
    public:
        gpu_ptr() : ptr_(nullptr), count_(0) {}
        gpu_ptr(std::size_t count) : ptr_(nullptr), count_(0)
        {
            allocate(count);
        }

        ~gpu_ptr()
        {
            release();
        }

        // Non-copyable
        gpu_ptr(const gpu_ptr&) = delete;
        gpu_ptr& operator=(const gpu_ptr&) = delete;

        // Movable
        gpu_ptr(gpu_ptr&& other) : ptr_(other.ptr_), count_(other.count_)
        {
            other.ptr_ = nullptr;
            other.count_ = 0;
        }

        gpu_ptr& operator=(gpu_ptr&& other)
        {
            release();
            ptr_ = other.ptr_;
            count_ = other.count_;

            other.ptr_ = nullptr;
            other.count_ = 0;

            return *this;
        }

        /** Allocate memory for @p count items
         * 
         * @param count Size of the array (number of items)
         */
        void allocate(std::size_t count)
        {
            release();
            CUCHECK(cudaMalloc((void**)&ptr_, count * sizeof(T)));
            count_ = count;
        }

        /** Free GPU memory held by this object
         */
        void release()
        {
            if (ptr_ != nullptr)
            {
                CUCHECK(cudaFree(ptr_));
                ptr_ = nullptr;
                count_ = 0;
            }
        }

        /** Get pointer to memory on the GPU
         * 
         * @returns pointer to memory in GPU memory space
         */
        T* get() const 
        {
            return ptr_;
        }

        /** Copy data from @p range to GPU 
         * 
         * @param range Values to copy
         */
        template<typename Range>
        void copy_to_gpu(Range&& range)
        {
            assert(ptr_ != nullptr);
            CUCHECK(cudaMemcpy(ptr_, &range[0], count_ * sizeof(T), cudaMemcpyHostToDevice));
        }

        /** Copy data to @p range from GPU 
         * 
         * @param range Destination
         */
        template<typename Range>
        void copy_from_gpu(Range&& range)
        {
            assert(ptr_ != nullptr);
            CUCHECK(cudaMemcpy(&range[0], ptr_, count_ * sizeof(T), cudaMemcpyDeviceToHost));
        }

    private:
        T* ptr_;
        std::size_t count_;
    };

    /** This class uses CUDA to solve the recipe assignment problem
     */
    class cuda_assignment_algorithm
    {
    public:
        // maximal number of equipment slots
        inline static constexpr std::size_t MAX_SLOT_COUNT = cuda::MAX_SLOT_COUNT;

        // Type used to index recipes during computation
        using recipe_index_t = std::uint8_t;
        // Recipe cost type
        using cost_t = recipe::cost_t;

        cuda_assignment_algorithm();

        // Non-copyable
        cuda_assignment_algorithm(const cuda_assignment_algorithm&) = delete;
        cuda_assignment_algorithm& operator=(const cuda_assignment_algorithm&) = delete;

        // Movable
        cuda_assignment_algorithm(cuda_assignment_algorithm&&) = default;
        cuda_assignment_algorithm& operator=(cuda_assignment_algorithm&&) = default;

        /** Allocate memory for the algorithm
         * 
         * @param max_res Maximal used resistances
         */
        void initialize(resistance max_res);

        /** Find assignment of @p recipes to equipment @p slots which minimizes cost and 
         * has at least @p required resistances.
         * 
         * @param required Required resistances 
         * @param slots Free equipment slots where we can apply recipes
         * @param recipes Available recipes
         * 
         * @return assignment of recipes to slots or invalid assingment object if assignment is not possible.
         */
        assignment run(
            resistance required, 
            const std::vector<recipe::slot_t>& slots, 
            const std::vector<recipe>& recipes);

    private:
        resistance max_res_;

        // CPU memory
        std::vector<cost_t> output_cost_;
        std::vector<recipe_index_t> output_assignment_;

        // GPU buffers
        gpu_ptr<cost_t> best_cost_;
        gpu_ptr<cost_t> next_best_cost_;
        gpu_ptr<recipe_index_t> best_assignment_;
        gpu_ptr<recipe_index_t> next_best_assignment_;
    };
}

#endif // RECAP_CUDA_ASSIGNMENT_ALGORITHM_HPP_