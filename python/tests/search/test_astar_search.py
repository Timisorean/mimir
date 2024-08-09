from pymimir import PDDLParser, PDDLFactories, Problem, LiftedApplicableActionGenerator, State, StateRepository, GroundAction, AStarAlgorithm, SearchStatus, IHeuristic, ConstSearchNode_double_double, DefaultAStarAlgorithmEventHandler, AStarAlgorithmEventHandlerBase

from pathlib import Path
from typing import List

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


class CustomBlindHeuristic(IHeuristic):
    def __init__(self):
        IHeuristic.__init__(self)  # Without this, a TypeError is raised.

    def compute_heuristic(self, state : State) -> float:
        return 0.


class CustomAStarAlgorithmEventHandler(AStarAlgorithmEventHandlerBase):
    def __init__(self, quiet = True):
        """
        AStarAlgorithmEventHandlerBase collects statistics and
        calls callback implementations only if quiet = False.
        """
        AStarAlgorithmEventHandlerBase.__init__(self, quiet)  # Without this, a TypeError is raised.

    def on_expand_state_impl(self, state : State, search_node: ConstSearchNode_double_double, problem : Problem, pddl_factories : PDDLFactories):
        pass

    def on_generate_state_impl(self, state : State, search_node: ConstSearchNode_double_double, problem : Problem, pddl_factories : PDDLFactories):
        pass

    def on_generate_state_relaxed_impl(self, state : State, search_node: ConstSearchNode_double_double, problem : Problem, pddl_factories : PDDLFactories):
        pass

    def on_generate_state_not_relaxed_impl(self, state : State, search_node: ConstSearchNode_double_double, problem : Problem, pddl_factories : PDDLFactories):
        pass

    def on_close_state_impl(self, state : State, search_node: ConstSearchNode_double_double, problem : Problem, pddl_factories : PDDLFactories):
        pass

    def on_finish_f_layer_impl(self, f_value: float, num_expanded_states : int, num_generated_states : int):
        pass

    def on_prune_state_impl(self, state : State, problem : Problem, pddl_factories : PDDLFactories):
        pass

    def on_start_search_impl(self, start_state : State, problem : Problem, pddl_factories : PDDLFactories):
        pass

    def on_end_search_impl(self):
        pass

    def on_solved_impl(self, ground_action_plan: List[GroundAction]):
        pass

    def on_unsolvable_impl(self):
        pass

    def on_exhausted_impl(self):
        pass


def test_astar_search():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = PDDLParser(domain_filepath, problem_filepath)
    lifted_applicable_action_generator = LiftedApplicableActionGenerator(parser.get_problem(), parser.get_pddl_factories())
    state_repository = StateRepository(lifted_applicable_action_generator)
    blind_heuristic = CustomBlindHeuristic()
    event_handler = CustomAStarAlgorithmEventHandler(False)
    astar_search_algorithm = AStarAlgorithm(lifted_applicable_action_generator, state_repository, blind_heuristic, event_handler)
    search_status, plan = astar_search_algorithm.find_solution()

    assert search_status == SearchStatus.SOLVED
    assert len(plan) == 3
