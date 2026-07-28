"""
Cue management for BeamCommander
"""
import json
import os
import logging
from typing import Dict, Optional
from .app_state import AppState, CueState

logger = logging.getLogger(__name__)


class CueManager:
    """
    Manages cue save/recall and persistence
    """
    
    def __init__(self, state: AppState, max_cues: int = 30):
        """
        Initialize cue manager
        
        Args:
            state: Application state reference
            max_cues: Maximum number of cues to support
        """
        self.state = state
        self.max_cues = max_cues
        self.cues: Dict[int, CueState] = {}
        self.cues_file = "cues.json"
    
    def save_cue(self, cue_num: int) -> bool:
        """
        Save current state to a cue slot
        
        Args:
            cue_num: Cue number (1-based)
            
        Returns:
            True if saved successfully
        """
        if cue_num < 1 or cue_num > self.max_cues:
            logger.warning(f"Invalid cue number: {cue_num}")
            return False
        
        try:
            cue = CueState.from_app_state(self.state)
            self.cues[cue_num] = cue
            logger.info(f"Saved cue {cue_num}")
            self.save_to_disk()
            return True
        except Exception as e:
            logger.error(f"Error saving cue {cue_num}: {e}")
            return False
    
    def recall_cue(self, cue_num: int) -> bool:
        """
        Recall a cue and apply it to current state
        
        Args:
            cue_num: Cue number (1-based)
            
        Returns:
            True if recalled successfully
        """
        if cue_num < 1 or cue_num > self.max_cues:
            logger.warning(f"Invalid cue number: {cue_num}")
            return False
        
        cue = self.cues.get(cue_num)
        if not cue or not cue.populated:
            logger.warning(f"Cue {cue_num} is empty")
            return False
        
        try:
            cue.apply_to_app_state(self.state)
            logger.info(f"Recalled cue {cue_num}")
            return True
        except Exception as e:
            logger.error(f"Error recalling cue {cue_num}: {e}")
            return False
    
    def save_to_disk(self, filepath: Optional[str] = None) -> bool:
        """
        Save all cues to disk
        
        Args:
            filepath: Optional custom filepath
            
        Returns:
            True if saved successfully
        """
        if filepath is None:
            filepath = self.cues_file
        
        try:
            data = {
                str(num): cue.to_dict()
                for num, cue in self.cues.items()
                if cue.populated
            }
            
            with open(filepath, 'w') as f:
                json.dump(data, f, indent=2)
            
            logger.info(f"Saved {len(data)} cues to {filepath}")
            return True
        except Exception as e:
            logger.error(f"Error saving cues to disk: {e}")
            return False
    
    def load_from_disk(self, filepath: Optional[str] = None) -> bool:
        """
        Load cues from disk
        
        Args:
            filepath: Optional custom filepath
            
        Returns:
            True if loaded successfully
        """
        if filepath is None:
            filepath = self.cues_file
        
        if not os.path.exists(filepath):
            logger.info(f"Cues file not found: {filepath}")
            return False
        
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)
            
            for num_str, cue_dict in data.items():
                try:
                    num = int(num_str)
                    if 1 <= num <= self.max_cues:
                        cue = CueState(**cue_dict)
                        self.cues[num] = cue
                except (ValueError, TypeError) as e:
                    logger.warning(f"Invalid cue data for {num_str}: {e}")
            
            logger.info(f"Loaded {len(self.cues)} cues from {filepath}")
            return True
        except Exception as e:
            logger.error(f"Error loading cues from disk: {e}")
            return False
    
    def clear_cue(self, cue_num: int) -> bool:
        """
        Clear a cue slot
        
        Args:
            cue_num: Cue number (1-based)
            
        Returns:
            True if cleared successfully
        """
        if cue_num in self.cues:
            del self.cues[cue_num]
            self.save_to_disk()
            logger.info(f"Cleared cue {cue_num}")
            return True
        return False
    
    def clear_all(self) -> bool:
        """
        Clear all cues
        
        Returns:
            True if cleared successfully
        """
        self.cues.clear()
        self.save_to_disk()
        logger.info("Cleared all cues")
        return True
