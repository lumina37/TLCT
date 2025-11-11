#pragma once

#include "tlct/config/concepts/arrange.hpp"
#include "tlct/convert/concepts/multiview.hpp"
#include "tlct/convert/concepts/patchsize.hpp"

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange_, concepts::CPsizeImpl TPsizeImpl_, typename TMvImpl_>
    requires concepts::CMvImpl<TMvImpl_, typename TPsizeImpl_::TBridge>
struct ManagerTraits_ {
    using TArrange = TArrange_;
    using TPsizeImpl = TPsizeImpl_;
    using TMvImpl = TMvImpl_;
};

}  // namespace tlct::_cvt
