//
// For the copyright information for this file, please search up the
// directory tree for the first COPYING file.
//

{
  let pfss = require("bindings")("pfss");
  for (let k in pfss) {
    global[k] = pfss[k];
  }
}

crypto = require("crypto")

function main(key1, key2) {

  const domain_bits = 32;
  const range_bits = 32;
  const range_mask = 0xFFFFFFFF;

  const alpha = new Uint32Array(1);
  alpha[0] = 12345;

  const beta = new Uint32Array(1);
  beta[0] = 67890;

  console.log();
  console.log("domain_bits = " + domain_bits);
  console.log("range_bits = " + range_bits);
  console.log("alpha = " + alpha[0]);
  console.log("beta = " + beta[0]);
  console.log();

  const key_blob_size = new Uint32Array(1);
  const rand_buf_size = new Uint32Array(1);

  console.log("pfss_gen_sizes:");
  {
    const s = pfss_gen_sizes(
        domain_bits, range_bits, key_blob_size, 0, rand_buf_size, 0);
    console.log("  status = " + pfss_get_status_name(s));
    if (s != PFSS_OK) {
      console.log();
      return;
    }
  }
  console.log("  key_blob_size = " + key_blob_size[0]);
  console.log("  rand_buf_size = " + rand_buf_size[0]);
  console.log();

  const key1_blob = new Uint8Array(key_blob_size[0]);
  const key2_blob = new Uint8Array(key_blob_size[0]);
  const rand_buf = new Uint8Array(rand_buf_size[0]);
  crypto.randomFillSync(rand_buf);

  console.log("pfss_gen:");
  {
    const s = pfss_gen(
        domain_bits,
        range_bits,
        new Uint8Array(alpha.buffer),
        0,
        alpha.byteLength,
        PFSS_NATIVE_ENDIAN,
        new Uint8Array(beta.buffer),
        0,
        beta.byteLength,
        PFSS_NATIVE_ENDIAN,
        key1_blob,
        0,
        key2_blob,
        0,
        rand_buf,
        0);
    console.log("  status = " + pfss_get_status_name(s));
    if (s != PFSS_OK) {
      console.log();
      return;
    }
  }
  console.log("  key1_blob = " + key1_blob);
  console.log("  key2_blob = " + key2_blob);
  console.log();

  console.log("pfss_parse_key key1:");
  {
    const s = pfss_parse_key(key1, key1_blob, 0, key1_blob.byteLength);
    console.log("  status = " + pfss_get_status_name(s));
    if (s != PFSS_OK) {
      console.log();
      return;
    }
  }
  console.log("  key1.id = " + key1.id);
  console.log();

  console.log("pfss_parse_key key2:");
  {
    const s = pfss_parse_key(key2, key2_blob, 0, key2_blob.byteLength);
    console.log("  status = " + pfss_get_status_name(s));
    if (s != PFSS_OK) {
      console.log();
      return;
    }
  }
  console.log("  key2.id = " + key2.id);
  console.log();

  console.log("pfss_eval:");
  const x = new Uint32Array(1);
  const y1 = new Uint32Array(1);
  const y2 = new Uint32Array(1);
  for (x[0] = alpha[0] - 1; x[0] != alpha[0] + 2; ++x[0]) {
    {
      const s = pfss_eval(
          key1,
          new Uint8Array(x.buffer),
          0,
          x.byteLength,
          PFSS_NATIVE_ENDIAN,
          new Uint8Array(y1.buffer),
          0,
          y1.byteLength,
          PFSS_NATIVE_ENDIAN);
      if (s != PFSS_OK) {
        console.log("  status = " + pfss_get_status_name(s));
        return;
      }
    }
    {
      const s = pfss_eval(
          key2,
          new Uint8Array(x.buffer),
          0,
          x.byteLength,
          PFSS_NATIVE_ENDIAN,
          new Uint8Array(y2.buffer),
          0,
          y2.byteLength,
          PFSS_NATIVE_ENDIAN);
      if (s != PFSS_OK) {
        console.log("  status = " + pfss_get_status_name(s));
        return;
      }
    }
    console.log(
        "  x = " + x[0] + ": (" + y1[0] + " + " + y2[0] + ") % 2^" +
        range_bits + " = " + ((y1[0] + y2[0]) & range_mask));
  }
  console.log();
}

const key1 = new Object();
const key2 = new Object();

main(key1, key2);

console.log("pfss_destroy_key(key1):");
{
  const s = pfss_destroy_key(key1);
  console.log("  status = " + pfss_get_status_name(s));
}
console.log();

console.log("pfss_destroy_key(key2):");
{
  const s = pfss_destroy_key(key2);
  console.log("  status = " + pfss_get_status_name(s));
}
console.log();
