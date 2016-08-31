#pragma once
#include "cmmn.h"

namespace plu {
	namespace memory {
		inline void* aligned_alloc(size_t size) {
#if defined(WIN32)
			return _aligned_malloc(size, 64);
#else
			return memalign(64, size);
#endif
		}
		template<typename T>
		inline T* aligned_alloc(size_t count = 1) {
			return (T*)aligned_alloc(sizeof(T)*count);
		}
		inline void aligned_free(void* p) {
#if defined(WIN32)
			_aligned_free(p);
#else
			free(p);
#endif	
		}

		struct arena {
			size_t cur_block_pos, block_size;
			char* current_block;
			vector<char*> used_blocks, available_blocks;
			arena(size_t bs = 32768) 
				: block_size(bs), cur_block_pos(0), current_block(aligned_alloc<char>(bs))
			{}

			void* allocate(size_t size) {
				size = ((size + 15) & (~15));
				if(cur_block_pos + size > block_size) {
					used_blocks.push_back(current_block);
					if(!available_blocks.empty() && size <= block_size) {
						current_block = available_blocks.back();
						available_blocks.pop_back();
					} else {
						current_block = aligned_alloc<char>(std::max(size, block_size));
					}
					cur_block_pos = 0;
				}
				void* p = current_block + cur_block_pos;
				cur_block_pos += size;
				return p;
			}
			template<typename T>
			T* allocate(size_t count = 1) {
				auto p = (T*)allocate(sizeof(T)*count);
				for(size_t i = 0; i < count; ++i)
					new (&p[i]) T();
				return p;
			}

			void free_all() {
				cur_block_pos = 0;
				while(!used_blocks.empty()) {
					available_blocks.push_back(used_blocks.back());
					used_blocks.pop_back();
				}
			}

			~arena() {
				aligned_free(current_block);
				for(auto p : used_blocks) aligned_free(p);
				for(auto p : available_blocks) aligned_free(p);
			}
		};
	}
}
