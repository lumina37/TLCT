#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts/neighbors.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/helper/neighbors.hpp"
#endif

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange>
NearNeighbors_<TArrange> NearNeighbors_<TArrange>::fromArrangeAndIndex(const TArrange& arrange,
                                                                       cv::Point index) noexcept {
    cv::Point leftIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point rightIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point upleftIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point uprightIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point downleftIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point downrightIdx{DEFAULT_INDEX, DEFAULT_INDEX};

    cv::Point2f selfPt = arrange.getMICenter(index);
    cv::Point2f leftPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f rightPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f upleftPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f uprightPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f downleftPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f downrightPt{DEFAULT_COORD, DEFAULT_COORD};

    if (index.x > 0) [[likely]] {
        leftIdx = {index.x - 1, index.y};
        leftPt = arrange.getMICenter(leftIdx);
    }
    if (index.x < (arrange.getMICols(index.y) - 1)) [[likely]] {
        rightIdx = {index.x + 1, index.y};
        rightPt = arrange.getMICenter(rightIdx);
    }

    const int isLeftRow = arrange.isOutShift() ^ (index.y % 2 == 0);  // this row is on the left side of up/down row
    const int udLeftXIdx = index.x - isLeftRow;                       // x index of the up/down-left MI
    const int udRightXIdx = udLeftXIdx + 1;                           // x index of the up/down-right MI

    if (index.y > 0) [[likely]] {
        const int yIdx = index.y - 1;
        if (udLeftXIdx >= 0) [[likely]] {
            upleftIdx = {udLeftXIdx, yIdx};
            upleftPt = arrange.getMICenter(upleftIdx);
        }
        if (udRightXIdx < arrange.getMICols(yIdx)) [[likely]] {
            uprightIdx = {udRightXIdx, yIdx};
            uprightPt = arrange.getMICenter(uprightIdx);
        }
    }

    if (index.y < (arrange.getMIRows() - 1)) [[likely]] {
        const int yIdx = index.y + 1;
        if (udLeftXIdx >= 0) [[likely]] {
            downleftIdx = {udLeftXIdx, yIdx};
            downleftPt = arrange.getMICenter(downleftIdx);
        }
        if (udRightXIdx < arrange.getMICols(yIdx)) [[likely]] {
            downrightIdx = {udRightXIdx, yIdx};
            downrightPt = arrange.getMICenter(downrightIdx);
        }
    }

    const TIndices indices{upleftIdx, uprightIdx, leftIdx, rightIdx, downleftIdx, downrightIdx};
    const TPoints points{upleftPt, uprightPt, leftPt, rightPt, downleftPt, downrightPt};

    return {indices, index, points, selfPt};
}

template <cfg::concepts::CArrange TArrange>
FarNeighbors_<TArrange> FarNeighbors_<TArrange>::fromArrangeAndIndex(const TArrange& arrange,
                                                                     cv::Point index) noexcept {
    cv::Point upIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point downIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point upleftIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point uprightIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point downleftIdx{DEFAULT_INDEX, DEFAULT_INDEX};
    cv::Point downrightIdx{DEFAULT_INDEX, DEFAULT_INDEX};

    cv::Point2f selfPt = arrange.getMICenter(index);
    cv::Point2f upPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f downPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f upleftPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f uprightPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f downleftPt{DEFAULT_COORD, DEFAULT_COORD};
    cv::Point2f downrightPt{DEFAULT_COORD, DEFAULT_COORD};

    const int isLeftRow = arrange.isOutShift() ^ (index.y % 2 == 0);  // this row is on the left side of up/down row
    const int udLeftXIdx = index.x - isLeftRow - 1;                   // x index of the up/down-left MI
    const int udRightXIdx = udLeftXIdx + 3;                           // x index of the up/down-right MI

    if (index.y > 0) [[likely]] {
        const int yIdx = index.y - 1;
        if (udLeftXIdx >= 0) [[likely]] {
            upleftIdx = {udLeftXIdx, yIdx};
            upleftPt = arrange.getMICenter(upleftIdx);
        }
        if (udRightXIdx < arrange.getMICols(yIdx)) [[likely]] {
            uprightIdx = {udRightXIdx, yIdx};
            uprightPt = arrange.getMICenter(uprightIdx);
        }

        if (index.y > 1) [[likely]] {
            upIdx = {index.x, index.y - 2};
            upPt = arrange.getMICenter(upIdx);
        }
    }

    if (index.y < (arrange.getMIRows() - 1)) [[likely]] {
        const int yIdx = index.y + 1;
        if (udLeftXIdx >= 0) [[likely]] {
            downleftIdx = {udLeftXIdx, yIdx};
            downleftPt = arrange.getMICenter(downleftIdx);
        }
        if (udRightXIdx < arrange.getMICols(yIdx)) [[likely]] {
            downrightIdx = {udRightXIdx, yIdx};
            downrightPt = arrange.getMICenter(downrightIdx);
        }

        if (index.y < (arrange.getMIRows() - 2)) [[likely]] {
            downIdx = {index.x, index.y + 2};
            downPt = arrange.getMICenter(downIdx);
        }
    }

    const TIndices indices{upIdx, upleftIdx, uprightIdx, downleftIdx, downrightIdx, downIdx};
    const TPoints points{upPt, upleftPt, uprightPt, downleftPt, downrightPt, downPt};

    return {indices, index, points, selfPt};
}

static_assert(concepts::CNeighbors<NearNeighbors_<cfg::CornersArrange>>);
template class NearNeighbors_<cfg::CornersArrange>;

static_assert(concepts::CNeighbors<NearNeighbors_<cfg::OffsetArrange>>);
template class NearNeighbors_<cfg::OffsetArrange>;

static_assert(concepts::CNeighbors<FarNeighbors_<cfg::CornersArrange>>);
template class FarNeighbors_<cfg::CornersArrange>;

static_assert(concepts::CNeighbors<FarNeighbors_<cfg::OffsetArrange>>);
template class FarNeighbors_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt
