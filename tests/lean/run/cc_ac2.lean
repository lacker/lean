open tactic

instance aa : is_associative ℕ (+) := ⟨nat.add_assoc⟩
instance ac : is_commutative ℕ (+) := ⟨nat.add_comm⟩
instance ma : is_associative ℕ (*) := ⟨nat.mul_assoc⟩
instance mc : is_commutative ℕ (*) := ⟨nat.mul_comm⟩

example (a b c d : nat) (f : nat → nat → nat) : b + a = d → f (a + b + c) a = f (c + d) a :=
by cc
