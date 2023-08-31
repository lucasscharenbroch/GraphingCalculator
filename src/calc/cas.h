#ifndef CAS
#define CAS

#include "backend.h"

// symbolic derivative options
extern string diff_id;
extern bool is_partial;

unique_ptr<TreeNode> symb_deriv(unique_ptr<TreeNode>&& node);
unique_ptr<TreeNode> symb_simp(unique_ptr<TreeNode>&& tree);

#endif // CAS
