#pragma once

#include "MatchKeySequence.h"
#include <functional>

struct Mapping {
  KeySequence input;
  KeySequence output;
};

struct MappingOverride {
  int mapping_index;
  KeySequence output;
};
using MappingOverrideSet = std::vector<MappingOverride>;

class Stage {
public:
  Stage(std::vector<Mapping> mappings,
        std::vector<MappingOverrideSet> override_sets);

  bool is_output_down() const { return !m_output_down.empty(); }
  const std::vector<Mapping>& mappings() const;
  const std::vector<MappingOverrideSet>& override_sets() const;
  const KeySequence& sequence() const { return m_sequence; }
  void activate_override_set(int index);
  KeySequence update(KeyEvent event);
  void reuse_buffer(KeySequence&& buffer);
  void validate_state(const std::function<bool(KeyCode)>& is_down);

private:
  std::pair<MatchResult, const Mapping*> find_mapping(
    ConstKeySequenceRange sequence, bool accept_might_match) const;
  void apply_input(KeyEvent event);
  void release_triggered(KeyCode key);
  const KeySequence& get_output(const Mapping& mapping) const;
  void forward_from_sequence();
  void apply_output(const KeySequence& expression, KeyCode trigger);
  void update_output(const KeyEvent& event, KeyCode trigger);
  void finish_sequence(ConstKeySequenceRange sequence);
  void output_current_sequence(const KeySequence& expression,
      KeyState state, KeyCode trigger);

  const std::vector<Mapping> m_mappings;
  const std::vector<MappingOverrideSet> m_override_sets;

  MatchKeySequence m_match;
  const MappingOverrideSet* m_active_override_set{ };

  // the input since the last match (or already matched but still hold)
  KeySequence m_sequence;
  bool m_sequence_might_match{ };

  // the keys which were output and are still down
  struct OutputDown {
    KeyCode key;
    KeyCode trigger;
    bool suppressed;           // by KeyState::Not event
    bool temporarily_released; // by KeyState::Not event
    bool pressed_twice;
  };
  std::vector<OutputDown> m_output_down;

  // temporary buffer
  KeySequence m_output_buffer;
  std::vector<KeyCode> m_toggle_virtual_keys;
};
