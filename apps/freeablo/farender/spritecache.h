#ifndef SPRITE_CACHE_H
#define SPRITE_CACHE_H

#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
#include <list>
#include <utility>
#include <atomic>

#include <render/render.h>

namespace FARender
{
    class Renderer;
    class SpriteCache;
    class FASpriteGroup
    {
        public:
            FASpriteGroup():
                animLength(1),
                width(1),
                height(1),
                spriteCacheIndex(0)
            {}
        
            bool isValid()
            {
                return spriteCacheIndex != 0;
            }

            uint32_t getAnimLength()
            {
                return animLength;
            }

            uint32_t getWidth()
            {
                return width;
            }

            uint32_t getHeight()
            {
                return height;
            }

            uint32_t getCacheIndex()
            {
                return spriteCacheIndex;
            }

        private:
            uint32_t animLength;
            uint32_t width;
            uint32_t height;
            uint32_t spriteCacheIndex;

        friend class SpriteCache;
        friend class SpriteManager;
    };

    struct TilesetPath
    {
        std::string celPath;
        std::string minPath;
        bool top;

        TilesetPath(std::string _c, std::string _m, bool _t)
            :celPath(_c), minPath(_m), top(_t) {}

        TilesetPath() {}
    };

    struct CacheEntry
    {
        Render::SpriteGroup* sprite;
        std::list<uint32_t>::iterator it;
        bool immortal;

        CacheEntry(Render::SpriteGroup* _sprite, std::list<uint32_t>::iterator _it, bool _immortal)
            :sprite(_sprite), it(_it), immortal(_immortal) {}

        CacheEntry() {}
    };

    ///
    /// @brief Multithread sprite cache
    ///
    /// FASpriteGroup is returned from the get() functions used in the game thread, which do not actually do image loading.
    /// get(uint32_t index) method is used to get a Render::SpriteGroup pointer in the render thread, actual image loading is done lazily here.
    /// The index value comes from FASpriteGroup.spriteCacheIndex
    ///
    class SpriteCache
    {
        public:
            SpriteCache(uint32_t size);
            ~SpriteCache();

            FASpriteGroup* get(const std::string& path); ///< To be called from the game thread

            /// Same as get(const std::string&), but for tileset sprites
            /// @brief To be called from the game thread
            FASpriteGroup* getTileset(const std::string& celPath, const std::string& minPath, bool top);

            uint32_t newUniqueIndex(); ///< Can be called from any thread

            /// Directly inserts a sprite into the cache at the specified index.
            /// Does not touch any of the m*toCache members, so the only way to get a reference is to have one from before
            /// @brief To be called from the render thread
            void directInsert(Render::SpriteGroup* sprite, uint32_t cacheIndex);

            /// Gets a Render::SpriteGroup* from the cache if it has been loaded before, otherwise load it,
            /// insert it into the cache, and then return it.
            ///
            /// @brief To be called from the render thread
            Render::SpriteGroup* get(uint32_t index);

            /// Used when we need to guarantee that a sprite will not be evicted for a period of time
            /// @brief To be called from the render thread
            void setImmortal(uint32_t index, bool immortal);

            /// @brief To be called from the game thread
            std::string getPathForIndex(uint32_t index);

            /// The only creation point for FASpriteGroups
            /// @brief To be called from the game thread
            FASpriteGroup* allocNewSpriteGroup();

            void clear(); //< To be called from the render thread

        private:
            void moveToFront(uint32_t index);
            void evict();

            std::map<std::string, FASpriteGroup*> mStrToCache;
            std::map<uint32_t, std::string> mCacheToStr;

            std::map<std::string, FASpriteGroup*> mStrToTilesetCache;
            std::map<uint32_t, TilesetPath> mCacheToTilesetPath;

            std::map<uint32_t, uint32_t> mCacheToSprite;

            std::map<uint32_t, CacheEntry> mCache;
            std::list<uint32_t> mUsedList;

            std::atomic<uint32_t> mNextCacheIndex;

            uint32_t mCurrentSize;
            uint32_t mMaxSize;

            static constexpr uint32_t SPRITEGROUP_STORE_BLOCK_SIZE = 256;
            std::vector<FASpriteGroup*> mSpriteGroupStore;
            uint32_t mSpriteGroupCurrentBlockIndex = 0;
    };
}

#endif
