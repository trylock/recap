#ifndef RECAP_CUDA_ASSIGNMENT_HPP_
#define RECAP_CUDA_ASSIGNMENT_HPP_

#include <iostream>
#include <cstdint>
#include <vector>
#include <cstdint>
#include <string>
#include <exception>

#include <cuda_runtime.h>

#include "recipe.hpp"
#include "resistance.hpp"
#include "assignment.hpp"
#include "assignment_kernel.hpp"
#include "assignment_algorithm.hpp"

namespace recap
{
    /** An error thrown if there is an error during CUDA API function execution
     */
    class cuda_error : public std::exception 
    {
    public: 
        inline cuda_error(cudaError_t result, int line, const char* file)
        {
            msg_ = std::string{"CUDA error at "} + file + std::string{":"} + 
                std::to_string(line) + " - " + cudaGetErrorString(result);
        }

        inline const char* what() const noexcept override 
        {
            return msg_.c_str();
        }

    private:
        std::string msg_;
    };

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
            throw cuda_error{ result, line, file };
        }
    }

    #define CUCHECK(result) (::recap::check_cuda_result((result), __LINE__, __FILE__))

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
         * @param count Number of items to copy
         */
        template<typename Range>
        void copy_to_gpu(Range&& range, std::size_t count)
        {
            if (count <= 0)
            {
                return;
            }
            assert(ptr_ != nullptr);
            assert(count <= count_);
            CUCHECK(cudaMemcpy(ptr_, &range[0], count * sizeof(T), cudaMemcpyHostToDevice));
        }

        /** Copy data from @p range to GPU 
         * 
         * @param range Values to copy
         */
        template<typename Range>
        void copy_to_gpu(Range&& range)
        {
            copy_to_gpu(std::forward<Range>(range), count_);
        }

        /** Copy data to @p range from GPU 
         * 
         * @param range Destination
         * @param count Number of items to copy
         */
        template<typename Range>
        void copy_from_gpu(Range&& range, std::size_t count)
        {
            if (count <= 0)
            {
                return;
            }
            assert(ptr_ != nullptr);
            assert(count <= count_);
            CUCHECK(cudaMemcpy(&range[0], ptr_, count * sizeof(T), cudaMemcpyDeviceToHost));
        }

        /** Copy data to @p range from GPU 
         * 
         * @param range Destination
         */
        template<typename Range>
        void copy_from_gpu(Range&& range)
        {
            copy_from_gpu(range, count_);
        }

    private:
        T* ptr_;
        std::size_t count_;
    };

    /** This class uses CUDA to solve the recipe assignment problem
     */
    class cuda_assignment : public assignment_algorithm
    {
    public:
        // maximal number of equipment slots
        inline static constexpr std::size_t MAX_SLOT_COUNT = cuda::MAX_SLOT_COUNT;

        // Type used to index recipes during computation
        using recipe_index_t = std::uint8_t;
        // Recipe cost type
        using cost_t = recipe::cost_t;

        cuda_assignment();
        virtual ~cuda_assignment() {}

        // Non-copyable
        cuda_assignment(const cuda_assignment&) = delete;
        cuda_assignment& operator=(const cuda_assignment&) = delete;

        // Movable
        cuda_assignment(cuda_assignment&&) = default;
        cuda_assignment& operator=(cuda_assignment&&) = default;

        /** Identifier of this algorithms
         * 
         * @returns name of this algorithm
         */
        const char* name() const override;

        /** Allocate memory for problem instances
         * 
         * @param max_resistances Maximal number of resistances
         * @param max_recipes Maximal number of recipes
         */
        void initialize(resistance max_resistances, std::size_t max_recipes) override;

        /** Find assignment of @p recipes to equipment @p slots which minimizes cost and 
         * has at least @p required resistances.
         * 
         * @param required Required resistances 
         * @param slots Free equipment slots where we can apply recipes
         * @param recipes Available recipes
         * 
         * @return assignment of recipes to slots or invalid assingment object if assignment is not possible.
         */
        assignment find_minimal_assignment(
            resistance required, 
            const std::vector<recipe::slot_t>& slots, 
            const std::vector<recipe>& recipes) override;

    private:
        // CPU memory
        std::vector<cost_t> output_cost_;
        std::vector<recipe_index_t> output_assignment_;
        std::vector<cost_t> buffer_cost_;
        std::vector<recipe::slot_t> buffer_slot_;
        std::vector<cuda::vector4<resistance::item_t>> buffer_resist_;

        // GPU buffers
        gpu_ptr<cost_t> best_cost_;
        gpu_ptr<cost_t> next_best_cost_;
        gpu_ptr<recipe_index_t> best_assignment_;
        gpu_ptr<recipe_index_t> next_best_assignment_;
        gpu_ptr<cost_t> recipe_cost_;
        gpu_ptr<recipe::slot_t> recipe_slot_;
        gpu_ptr<cuda::vector4<resistance::item_t>> recipe_resist_;

        /** Add recipes to input data
         * 
         * @param input Input data
         * @param recipes Available recipes
         */
        void set_recipes(cuda::input_data& input, const std::vector<recipe>& recipes);

        /** Set table size and table dimensions in @p input
         * 
         * @param input Input data
         * @param req Required resistances
         */
        void set_table_size(cuda::input_data& input, resistance req);

        /** Initialize best cost in @p input data
         * 
         * @param input Input dat
         * @param value_count Table size
         */
        void set_table_buffers(cuda::input_data& input, std::size_t value_count);
    };
}

#endif // RECAP_CUDA_ASSIGNMENT_HPP_