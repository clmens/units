````markdown
```markdown
# Research notes: Self‑rewiring / self‑improving neural network topology

This short overview summarizes approaches and considerations for neural networks that change their topology or connectivity over time — often described as self‑rewiring, structural plasticity, or topology learning. The goal here is to provide accessible background for the repository's documentation.

1) Key concepts
- Structural plasticity: biological and algorithmic mechanisms by which neurons and synapses are created, pruned, or strengthened over time.
- Self‑rewiring: algorithms that alter network connectivity (add/remove edges, rewire connections) as part of training or during network operation.
- Neuroevolution: evolutionary algorithms that evolve network weights and structure (e.g., NEAT — NeuroEvolution of Augmenting Topologies).
- Dynamic sparse training: maintain a sparse connectivity pattern during training, periodically pruning low-importance weights and growing new ones (e.g., RigL-like methods).
- Differentiable architecture search: optimize architecture parameters with gradient-based methods (DARTS family), sometimes combined with operations that change connectivity.
- Plasticity rules: local update rules (Hebbian, STDP) that adapt connection strengths based on activity patterns; can be extended with structural rules that create/prune synapses.

2) Common mechanisms for self‑rewiring
- Prune-and-grow cycles:
  - Periodically prune low-magnitude or low-importance weights and regrow connections (random or targeted).
  - Advantages: keeps model compact; can discover efficient topologies.
- Neuroevolution:
  - Use genetic algorithms to mutate and crossover both weights and graph structure.
  - NEAT introduced historical markings and complexification (start small and add nodes/edges).
- Gradient-friendly structural updates:
  - Make topology parameters continuous and apply gradient-based optimization (e.g., soft masks, concrete distributions).
  - After optimization, discretize to obtain a final sparse topology.
- Local rules and homeostasis:
  - Use biologically-inspired local rules (Hebbian, anti-Hebbian) combined with normalization or resource constraints to regulate connection density.
- Meta-learning / learned rewiring:
  - A meta-learner observes learning progress and recommends topology changes; can be trained to produce growth/prune policies.

3) Practical algorithmic patterns
- Initialize small, grow as needed — start with a compact network and add capacity where the error is high.
- Use importance metrics for pruning: weight magnitude, gradient-based saliency, second-order approximations (diagonal Fisher / Hessian approximations).
- When regrowing connections, consider targeted growth (connect to high-activation neurons) rather than random growth for faster convergence.
- Maintain stability: frequent topology changes can destabilize training; use slow rewiring schedules and/or replay mechanisms to preserve knowledge.
- Combine with continual learning techniques to reduce catastrophic forgetting when structure changes.

4) Theoretical and practical tradeoffs
- Computation and memory: dynamic structures complicate efficient batched matrix multiplies; sparse and dynamic frameworks are improving but more complex than dense training.
- Credit assignment: structural changes can make gradient-based credit assignment noisy; methods that incorporate smooth or probabilistic rewiring can help.
- Evaluation complexity: discovering beneficial topologies requires careful validation and sometimes long training runs or population-based search.
- Interpretability: self‑rewiring networks may reveal sparse motifs or circuits, but interpreting why a particular topology emerged can be nontrivial.

5) Research directions and ideas you might try
- Combine local plasticity (Hebbian-like) with global optimization: let weights adapt quickly via local rules, and let a slower outer loop prune/regrow connections using global signals (loss, validation performance).
- Hybrid neuroevolution + gradient descent: evolve topology at population timescales and train weights with gradient descent between topology changes.
- Dynamic sparse training at scale: test RigL-like schedules on large nets to see whether rewiring improves sample efficiency.
- Differentiable structural parameters: learn masks or continuous topology variables, then discretize once converged.

6) Practical references (keywords to search)
- NEAT / Neuroevolution of Augmenting Topologies
- RigL (sparse training with dynamic rewiring)
- DARTS (Differentiable ARchiTecture Search)
- Hebbian learning, STDP, structural plasticity
- Lottery Ticket Hypothesis and pruning/regrowth methods

Short conclusion
Self‑rewiring networks sit at the intersection of biological inspiration and practical engineering tradeoffs. Effective designs typically combine local rules, careful scheduling for structural changes, and evaluation strategies that control instability. For exploratory projects, start small (e.g., toy networks, controlled pruning/regrowth cycles) and progressively scale to larger models once your rewiring policy shows stable improvements.
```
````