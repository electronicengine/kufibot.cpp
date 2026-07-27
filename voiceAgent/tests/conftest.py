"""Pytest configuration – adds voiceAgent/ to sys.path for direct imports."""
import sys
import os

# voiceAgent/tests/conftest.py -> voiceAgent/ is the parent directory
_voice_agent_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if _voice_agent_dir not in sys.path:
    sys.path.insert(0, _voice_agent_dir)
