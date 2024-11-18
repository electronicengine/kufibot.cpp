
#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#include "final/final.h"
#include <utility>
#include <deque>


class SubWindow : public finalcut::FDialog
{
  public:
    explicit SubWindow (finalcut::FWidget* parent= nullptr);

    SubWindow (const SubWindow&) = delete;

    SubWindow (SubWindow&&) noexcept = delete;

    ~SubWindow() noexcept override;

    auto operator = (const SubWindow&) -> SubWindow& = delete;

    auto operator = (SubWindow&&) noexcept -> SubWindow& = delete;

    template <typename InstanceT, typename CallbackT, typename... Args>
    void add_clicked_callback (finalcut::FWidget* widget, InstanceT&& instance, CallbackT&& callback, Args&&... args){
        widget->addCallback
        (
          "clicked",
          std::bind ( std::forward<CallbackT>(callback)
                    , std::forward<InstanceT>(instance)
                    , std::forward<Args>(args)... )
        );
    }

  protected:
    void onClose (finalcut::FCloseEvent*) override;
    void onShow  (finalcut::FShowEvent*) override;
    void adjustSize() override;
    void activate_window (finalcut::FDialog* win) const;

};

#endif // GRAPHWINDOW_H