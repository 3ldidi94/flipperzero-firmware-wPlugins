import fs  from 'fs';
import path  from 'path';
import { convertToString, prepareBinary,  NUM_COLUMNS } from './stringConverter'

function getAllFiles(dirPath, prefix) {
    const files = fs.readdirSync(dirPath);
    const fileList = [];
    files.forEach((file) => {
      const filePath = path.join(dirPath, file);
      const stat = fs.statSync(filePath);
  
      if (stat.isDirectory()) {
        fileList.push(...getAllFiles(filePath, prefix)); // Recursivamente agregar archivos de subcarpetas.
      } else {
        // if(filePath.match(/\.(html|ico|png|jpg)$/g) && !filePath.match(/\.DS_Store/g)) {
        if(filePath.match(/\.(gz)$/g) && !filePath.match(/\.DS_Store/g)) {
          const meta = {
              size: stat.size,
              filename: path.basename(filePath),
              path: (filePath.slice(filePath.indexOf('dist/') + 5)).replace(/-/g,'_').replace(/\//g,'_').replace(/\./g,'_')
          };
          const binBinaryFileHolder = fs.readFileSync(filePath);
          const byteArray = prepareBinary(binBinaryFileHolder);
          const convertedData = convertToString(byteArray, NUM_COLUMNS, meta.path, prefix);
          fileList.push(convertedData)
        }
      }
    });
    return fileList;
  }

export default function GenerateCHeaders(options = {filename : 'gameboy_server_assets', prefix: ''}) {

    const { filename, prefix } = options;

    return {
      name: 'file-list-plugin',
      generateBundle(_, bundle) {
        const distPath = path.resolve(__dirname, '../dist');
        const outputFile = path.resolve(__dirname , `../../../${filename}.h`);
        const fileList = getAllFiles(distPath, prefix).join('\n');
        fs.writeFileSync(outputFile, `#ifndef ${filename}_h\n#define ${filename}_h\n\n${fileList}\n#endif`, 'utf-8');
      },
    };
};