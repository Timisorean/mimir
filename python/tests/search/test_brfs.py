from pymimir import PDDLParser
from pymimir import SSG, LiftedAAG, MinimalEventHandler, DebugEventHandler, BrFsAlgorithm, SearchStatus

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()


def test_brfs():
    """ Test parsing a PDDL domain and problem file.
    """
    domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
    problem_filepath = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
    parser = PDDLParser(domain_filepath, problem_filepath)
    aag = LiftedAAG(parser.get_problem(), parser.get_factories())
    event_handler = DebugEventHandler()
    brfs = BrFsAlgorithm(aag, event_handler)
    search_status, plan = brfs.find_solution()

    assert search_status == SearchStatus.SOLVED
    assert len(plan) == 3
