# czlite v0.3.8 release inputs

The GitHub release publishes the verified microSD image and corresponding userspace sources as downloadable files. To verify the inputs locally, join the source archive parts and check both files.

```sh
cat source-part-* > czlite-v0.3.8-sources.tar.gz
sha256sum -c SHA256SUMS
```
