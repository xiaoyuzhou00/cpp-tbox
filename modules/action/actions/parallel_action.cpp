#include "parallel_action.h"

#include <algorithm>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace action {

using namespace std::placeholders;

ParallelAction::~ParallelAction() {
  for (auto action : children_)
    delete action;
}

int ParallelAction::append(Action *action) {
  assert(action != nullptr);

  if (std::find(children_.begin(), children_.end(), action) == children_.end()) {
    int index = children_.size();
    children_.push_back(action);
    action->setFinishCallback(std::bind(&ParallelAction::onChildFinished, this, index, _1));
    return index;
  } else {
    LogWarn("can't add child twice");
    return -1;
  }
}

bool ParallelAction::onStart() {
  succ_set_.clear();
  fail_set_.clear();

  for (Action *action : children_)
    action->start();
  return true;
}

bool ParallelAction::onStop() {
  for (Action *action : children_)
    action->stop();
  return true;
}

void ParallelAction::onChildFinished(int index, bool is_succ) {
  if (is_succ)
    succ_set_.insert(index);
  else
    fail_set_.insert(index);

  if (succ_set_.size() == children_.size()) {
    finish(true);
  } else if ((succ_set_.size() + fail_set_.size()) == children_.size()) {
    finish(false);
  }
}

}
}