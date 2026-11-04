"""Build-only GitHub token scope for refreshing community dictionaries."""
import os
import unittest
from unittest import mock
from urllib.request import Request

import build_synth


class DictionaryBuildAuthentication(unittest.TestCase):
    def test_token_is_limited_to_unredirected_github_api_requests(self):
        with mock.patch.dict(os.environ, {"RELOQ_BUILD_GITHUB_TOKEN": "test-token"}):
            with mock.patch.object(build_synth, "urlopen", return_value=object()) as fetch:
                api = Request("https://api.github.com/repos/example/example/branches/master")
                raw = Request("https://raw.githubusercontent.com/example/example/master/file")
                impostor = Request("https://api.github.com.example.org/file")
                insecure = Request("http://api.github.com/file")
                for req in (api, raw, impostor, insecure):
                    build_synth._build_dictionary_fetch(req, timeout=8)
                self.assertEqual(fetch.call_count, 4)
                self.assertEqual(api.get_header("Authorization"), "Bearer test-token")
                self.assertNotIn("Authorization", api.headers)
                self.assertEqual(api.unredirected_hdrs["Authorization"], "Bearer test-token")
                for req in (raw, impostor, insecure):
                    self.assertIsNone(req.get_header("Authorization"))

    def test_no_token_preserves_anonymous_requests(self):
        with mock.patch.dict(os.environ, {}, clear=True):
            with mock.patch.object(build_synth, "urlopen", return_value=object()):
                req = Request("https://api.github.com/repos/example/example/branches/master")
                build_synth._build_dictionary_fetch(req, timeout=8)
                self.assertIsNone(req.get_header("Authorization"))


if __name__ == "__main__":
    unittest.main()
