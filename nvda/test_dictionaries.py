"""Pinned data integrity, latest-master refresh and asynchronous setting races."""
from pathlib import Path
import io,json,sys,tempfile,threading,unittest
from urllib.error import URLError
from _reloq_dictionaries import *
from _reloq_dictionaries import _blob
R=Path(__file__).resolve().parents[1]
class Response(io.BytesIO):
    pass
class Server:
    def __init__(self):
        self.commit='1'*40;self.calls=[];self.corrupt=None;self.block=None
        self.data={name:(R/'data/community'/name).read_bytes()for name in FILES}
    def __call__(self,request,timeout):
        url=request.full_url;self.calls.append(url)
        if url.endswith('/branches/master'):
            if self.block:self.block.wait(2)
            value=json.dumps({'commit':{'sha':self.commit}}).encode()
        elif '/git/trees/'in url:
            value=json.dumps({'tree':[{'path':n,'type':'blob','size':len(b),'sha':_blob(b)}for n,b in self.data.items()]}).encode()
        else:
            name=url.rsplit('/',1)[-1];value=self.data[name]
            if name==self.corrupt:value+=b'corruption'
        return Response(value)
class Dictionaries(unittest.TestCase):
    def test_current_master_and_all_volumes(self):
        server=Server()
        with tempfile.TemporaryDirectory()as t:
            first=fetch_master(t,fetch=server);self.assertEqual(first.name,'1'*40)
            self.assertEqual(validate_snapshot(first)['commit'],server.commit)
            self.assertTrue(all((first/n).read_bytes()==b for n,b in server.data.items()))
            before=len(server.calls);fetch_master(t,fetch=server)
            self.assertEqual(len(server.calls)-before,1) # Master is re-resolved, verified data reused.
            server.commit='2'*40
            latest=fetch_master(t,fetch=server);self.assertEqual(latest.name,'2'*40)
            self.assertEqual(json.loads((Path(t)/'current.json').read_text())['commit'],server.commit)
    def test_failed_set_never_replaces_complete_set(self):
        server=Server()
        with tempfile.TemporaryDirectory()as t:
            fetch_master(t,fetch=server);previous=(Path(t)/'current.json').read_bytes()
            server.commit='3'*40;server.corrupt='ENURoot.dic'
            with self.assertRaises(ValueError):fetch_master(t,fetch=server)
            self.assertEqual((Path(t)/'current.json').read_bytes(),previous)
            self.assertFalse((Path(t)/server.commit).exists())
    def test_off_during_update_discards_late_completion(self):
        server=Server();server.block=threading.Event();delivered=[];errors=[]
        with tempfile.TemporaryDirectory()as t:
            update=CommunityDictionaries(R/'data/community',t,lambda rev,p:delivered.append((rev,p)),errors.append,fetch=server)
            update.set_enabled(True);self.assertEqual(len(delivered),1)
            update.set_enabled(False);self.assertIsNone(delivered[-1][1])
            server.block.set()
            # Join only test-owned updater threads; no speech thread is blocked.
            for thread in threading.enumerate():
                if thread.name=='Eloquence dictionary update':thread.join(5)
            self.assertEqual(len(delivered),2);self.assertFalse(errors)
            update.close()
    def test_offline_uses_bundle_without_waiting(self):
        ready=threading.Event();delivered=[];errors=[]
        def offline(*a,**k):raise URLError('offline')
        with tempfile.TemporaryDirectory()as t:
            u=CommunityDictionaries(R/'data/community',t,lambda rev,p:delivered.append(p),lambda e:(errors.append(e),ready.set()),fetch=offline)
            u.set_enabled(True);self.assertEqual(delivered,[R/'data/community'])
            self.assertTrue(ready.wait(2));self.assertEqual(len(errors),1);u.close()
    def test_incomplete_cache_falls_back_to_verified_bundle(self):
        with tempfile.TemporaryDirectory()as t:
            p=Path(t);(p/'current.json').write_text('{"commit":"'+'f'*40+'"}')
            ready=threading.Event();paths=[]
            def offline(*a,**k):raise URLError('offline')
            u=CommunityDictionaries(R/'data/community',p,lambda r,d:paths.append(d),lambda e:ready.set(),fetch=offline)
            u.set_enabled(True);self.assertEqual(paths,[R/'data/community']);self.assertTrue(ready.wait(2));u.close()
class LanguagePersistence(unittest.TestCase):
    def test_cancel_does_not_wait_for_native_close(self):
        from _reloq_languages import LanguageEngine
        closing, release, cancelled = threading.Event(), threading.Event(), threading.Event()
        holder, errors = [], []
        class Engine:
            def __init__(self, *args, **kwargs):pass
            def set_parameter(self, *args):pass
            def cancel(self):pass
            def close(self):
                closing.set()
                if not release.wait(5):raise AssertionError('close was never released')
        def owner():
            try:
                engine = LanguageEngine('eci.dll', engine_factory=Engine)
                holder.append(engine)
                engine.close()
            except BaseException as error:errors.append(error)
        worker = threading.Thread(target=owner)
        worker.start()
        self.assertTrue(closing.wait(2))
        caller = threading.Thread(target=lambda: (holder[0].cancel(), cancelled.set()))
        caller.start()
        try:self.assertTrue(cancelled.wait(1), 'cancel waited for native destruction')
        finally:
            release.set();caller.join(5);worker.join(5)
        self.assertFalse(errors)

    def test_existing_future_voices_and_atomic_rollback(self):
        from _reloq_languages import LanguageEngine, GERMAN, US_ENGLISH
        class Engine:
            instances = []
            def __init__(self, dll, **options):
                self.language = options["dialect"]
                self.path = None
                self.closed = False
                self.instances.append(self)
            def set_parameter(self, key, value):pass
            def set_dictionary_path(self, path):
                if path == "broken" and self.language == US_ENGLISH.id:
                    raise RuntimeError("failed native set")
                self.path = path
            def cancel(self):pass
            def close(self):self.closed = True
        e = LanguageEngine("eci.dll", languages={GERMAN.id: GERMAN, US_ENGLISH.id: US_ENGLISH}, engine_factory=Engine)
        e.set_dictionary_path("one")
        e.select_voice(US_ENGLISH.id, "1")
        e.select_voice(GERMAN.id, "2")
        self.assertEqual([x.path for x in Engine.instances], ["one"] * 2)
        e.set_dictionary_path("two")
        with self.assertRaises(RuntimeError):e.set_dictionary_path("broken")
        self.assertEqual([x.path for x in Engine.instances], ["two"] * 2)
        e.set_dictionary_path(None)
        self.assertEqual([x.path for x in Engine.instances], [None] * 2)
        e.close()
        self.assertTrue(all(x.closed for x in Engine.instances))

if __name__=='__main__':unittest.main()
