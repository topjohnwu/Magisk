import unittest

from quest_zygisk_patch import PatchError, detect_runtime_slot, normalized_slot


class FakeAdb:
    def __init__(self, replies):
        self.replies = replies

    def shell(self, command):
        return self.replies.get(command, "")


class SlotDetectionTests(unittest.TestCase):
    def test_normalizes_android_slot_forms(self):
        self.assertEqual(normalized_slot("_a\n"), "a")
        self.assertEqual(normalized_slot("b"), "b")
        self.assertEqual(normalized_slot("0"), "a")
        self.assertEqual(normalized_slot("1"), "b")
        self.assertIsNone(normalized_slot("unknown"))

    def test_runtime_sources_agree(self):
        adb = FakeAdb(
            {
                "getprop ro.boot.slot_suffix": "_b",
                "getprop ro.boot.slot": "b",
            }
        )
        slot, _ = detect_runtime_slot(adb)
        self.assertEqual(slot, "b")

    def test_runtime_disagreement_aborts(self):
        adb = FakeAdb(
            {
                "getprop ro.boot.slot_suffix": "_a",
                "getprop ro.boot.slot": "b",
            }
        )
        with self.assertRaises(PatchError):
            detect_runtime_slot(adb)

    def test_missing_slot_aborts(self):
        with self.assertRaises(PatchError):
            detect_runtime_slot(FakeAdb({}))


if __name__ == "__main__":
    unittest.main()
